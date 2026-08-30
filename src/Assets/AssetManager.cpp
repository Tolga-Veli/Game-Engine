#include "Assets/AssetManager.hpp"

#include "Core/Logging.hpp"
#include "Core/Assert.hpp"

#include "Assets/Traits/TextureTraits.hpp"
#include "Assets/Traits/ShaderTraits.hpp"
#include "Assets/Traits/MeshTraits.hpp"
#include "Assets/Traits/MaterialTraits.hpp"
#include "Assets/Traits/FontTraits.hpp"

#include <mutex>
#include <thread>

namespace Jupiter::Assets {
void AssetManager::Init() {
  m_RenderThreadID = std::this_thread::get_id();

  fs::path base = fs::current_path();
  if (!fs::exists(base / "assets")) {
    LOG_ERROR("AssetManager: FAILED to find assets folder!");
    return;
  }

  m_AssetsDir = (base / "assets").lexically_normal();

  LOG_INFO("AssetManager: Root established at {}", m_AssetsDir.string());

  m_WatcherThread =
      std::jthread([this](std::stop_token st) { WatcherThreadMain(st); });
}

void AssetManager::ProcessRenderTasks() {
  CORE_ASSERT(IsRenderThread(), "AssetManager::ProcessRenderTasks must run "
                                "on the render thread");

  std::vector<RenderTask> tasks;
  {
    std::scoped_lock lock(m_RenderTasksMutex);
    tasks.swap(m_RenderTasks);
  }

  for (auto &task : tasks)
    task();
}

// WARNING:
// Pool destruction happens here while the graphics context is still alive

void AssetManager::Shutdown() noexcept {
  CORE_ASSERT(IsRenderThread(), "AssetManager::Shutdown must run "
                                "on the render thread");

  m_WatcherThread.request_stop();

  if (m_WatcherThread.joinable())
    m_WatcherThread.join();

  ProcessRenderTasks();

  {
    // IMPORTANT:
    std::unique_lock lock(m_Mutex);
    m_Registry.clear();
    m_AssetPools.Clear();
  }

  {
    std::scoped_lock lock(m_RenderTasksMutex);
    m_RenderTasks.clear();
  }

  {
    std::scoped_lock lock(m_PendingMutex);
    m_PendingReloads.clear();
  }

  LOG_INFO("AssetManager: Resources cleared.");
}

void AssetManager::PollHotReload() {
  CORE_ASSERT(IsRenderThread(), "AssetManager::PollHotReload must run "
                                "on the render thread");

  std::vector<PendingReload> jobs;
  {
    std::scoped_lock lock(m_PendingMutex);

    if (m_PendingReloads.empty())
      return;

    jobs.swap(m_PendingReloads);
  }

  for (const auto &job : jobs) {
    using enum AssetType;
    switch (job.type) {
    case Shader:
      ReloadHelper<Rendering::Shader>(job);
      break;
    case Texture:
      ReloadHelper<Rendering::Texture>(job);
      break;
    case Font:
      ReloadHelper<Rendering::Font>(job);
      break;
    case Mesh:
      ReloadHelper<Rendering::Mesh>(job);
      break;
    case Material:
      ReloadHelper<Rendering::Material>(job);
      break;
    case Audio:
      CORE_ASSERT(false, "not implemented yet");
      break;
    }
  }
}

void AssetManager::WatcherThreadMain(std::stop_token st) {
  struct Tracked {
    AssetID ID;
    AssetType Type;
    fs::path Path;
    fs::file_time_type LastWriteTime;
  };

  while (!st.stop_requested()) {
    std::vector<Tracked> snapshot;
    {
      std::shared_lock lock(m_Mutex);
      snapshot.reserve(m_Registry.size());
      for (const auto &[id, record] : m_Registry) {
        if (record.runtime_created || record.path.empty())
          continue;

        snapshot.push_back(
            {id, record.type, record.path, record.last_write_time});
      }
    }

    for (const auto &tracked : snapshot) {
      if (st.stop_requested())
        break;

      std::error_code ec;
      const auto current = fs::last_write_time(tracked.Path, ec);
      if (ec || current == tracked.LastWriteTime)
        continue;

      bool shouldQueue = false;
      {
        std::unique_lock lock(m_Mutex);
        auto it = m_Registry.find(tracked.ID);

        if (it != m_Registry.end())
          continue;

        if (it->second.last_write_time != current) {
          it->second.last_write_time = current;
          shouldQueue = true;
        }
      }

      if (!shouldQueue)
        continue;

      {
        std::scoped_lock pendingLock(m_PendingMutex);
        m_PendingReloads.push_back({tracked.Type, tracked.ID, tracked.Path});
      }

      if (m_RendererWakeup)
        m_RendererWakeup();
    }
    std::this_thread::sleep_for(m_PollInterval);
  }
}

template <Asset T>
AssetHandle<T>
AssetManager::GetAssetHelper(const fs::path &relativePath,
                             const typename AssetTraits<T>::LoadOptions &opts) {
  CORE_ASSERT(IsRenderThread(),
              "Asset loading must be ran on the render thread");

  const fs::path path = (m_AssetsDir / relativePath).lexically_normal();
  std::string name = relativePath.generic_string();

  const AssetID id = AssetTraits<T>::MakeID(path, opts);
  {
    std::shared_lock lock(m_Mutex);
    if (auto it = m_Registry.find(id); it != m_Registry.end())
      return AssetHandle<T>{it->second.pool_idx, it->second.pool_gen};
  }

  auto value = AssetTraits<T>::LoadFromFile(path, opts);
  if (!value) {
    LOG_ERROR("AssetManager: failed to load {} '{}'", AssetTraits<T>::Str,
              path.string());

    return {};
  }

  std::error_code ec;
  const auto writeTime = fs::last_write_time(path, ec);

  std::unique_lock lock(m_Mutex);
  if (auto it = m_Registry.find(id); it != m_Registry.end())
    return AssetHandle<T>{it->second.pool_idx, it->second.pool_gen};

  auto &pool = m_AssetPools.GetPool<T>();
  AssetHandle<T> handle = pool.Insert(std::move(value));

  m_Registry.emplace(id, AssetRecord{AssetTraits<T>::Type, name, path,
                                     writeTime, handle.idx, handle.gen,
                                     std::move(opts), false});

  LOG_INFO("AssetManager: loaded {} '{}'", AssetTraits<T>::Str, name);
  return handle;
}

template <Asset T>
Ref<T> AssetManager::ResolveHelper(const AssetHandle<T> &handle) const {
  std::shared_lock lock(m_Mutex);
  const auto *value = m_AssetPools.GetPool<T>().Resolve(handle);
  return value ? *value : Ref<T>{};
}

// Reserve+register happen synchronously so concurrent Create() calls for
// the same name still dedupe correctly regardless of which thread they
// come from. factory() itself is queued and only ever runs later, on the
// render thread, out of ProcessRenderTasks().
template <Asset T, class Factory>
AssetHandle<T> AssetManager::CreateHelper(std::string_view name,
                                          Factory &&factory) {
  const AssetID id = HashRuntimeAssetName(name, AssetTraits<T>::Type);

  {
    std::shared_lock lock(m_Mutex);
    if (auto it = m_Registry.find(id); it != m_Registry.end())
      return AssetHandle<T>{it->second.pool_idx, it->second.pool_gen};
  }

  std::string nameStr(name);
  AssetHandle<T> handle;
  {
    std::unique_lock lock(m_Mutex);

    // someone may have beaten us to it between the two locks above
    if (auto it = m_Registry.find(id); it != m_Registry.end())
      return AssetHandle<T>{it->second.pool_idx, it->second.pool_gen};

    handle = m_AssetPools.GetPool<T>().Reserve();
    m_Registry.emplace(id, AssetRecord{AssetTraits<T>::Type,
                                       nameStr,
                                       {},
                                       {},
                                       {},
                                       handle.idx,
                                       handle.gen,
                                       true});
  }

  QueueRenderTask([this, id, handle, nameStr,
                   factory = std::forward<Factory>(factory)]() mutable {
    // IMPORTANT:
    // GPU creation happens here, on the render thread, whenever
    // ProcessRenderTasks() next picks this task up.
    Ref<T> value = factory();

    std::unique_lock lock(m_Mutex);

    // Shutdown() may have cleared the pools out from under this task
    if (!m_AssetPools.GetPool<T>().IsValid(handle))
      return;

    if (!value) {
      LOG_ERROR("AssetManager: failed to create runtime {} '{}'",
                AssetTraits<T>::Str, nameStr);

      m_AssetPools.GetPool<T>().Release(handle);
      m_Registry.erase(id);
      return;
    }

    m_AssetPools.GetPool<T>().Replace(handle, std::move(value));

    LOG_INFO("AssetManager: created runtime {} '{}'", AssetTraits<T>::Str,
             nameStr);
  });

  return handle;
}

template <Asset T> void AssetManager::ReloadHelper(const PendingReload &job) {
  CORE_ASSERT(IsRenderThread(), "Asset reload must run on render thread");

  AssetRecord record;
  {
    std::shared_lock lock(m_Mutex);
    auto it = m_Registry.find(job.id);
    if (it == m_Registry.end())
      return;

    record = it->second;
  }

  if (record.runtime_created || record.path.empty())
    return;

  auto opts =
      std::get<typename AssetTraits<T>::LoadOptions>(record.load_options);
  auto reloaded =
      CreateAssetFromImpl<T>(AssetTraits<T>::LoadFromFile(job.path, opts));

  if (!reloaded) {
    LOG_ERROR("AssetManager: hot reload FAILED for {} '{}'",
              to_string(AssetTraits<T>::type), job.path.string());

    return;
  }

  {
    std::unique_lock lock(m_Mutex);
    auto it = m_Registry.find(job.id);
    if (it == m_Registry.end())
      return;

    AssetHandle<T> handle{it->second.pool_idx, it->second.pool_gen};
    if (!m_AssetPools.GetPool<T>().Replace(handle, std::move(reloaded)))
      return;
  }

  LOG_INFO("AssetManager: reloaded {} '{}'", to_string(AssetTraits<T>::type),
           record.name);
}

} // namespace Jupiter::Assets
