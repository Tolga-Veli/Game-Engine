#pragma once

#include "Core/Utils.hpp"

#include "Traits.hpp"
#include "Types.hpp"
#include "Pool.hpp"

#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <mutex>

// IDEA:
// Game thread owns AssetHandle<T>
// pass to the render queue the creation of the resource
// render thread ONLY calls AssetManager::Resolve(handle)
//
// AssetHandle<T> // safe to pass between threads (since its easily copyable id
// of the actual resource)
//
// Ref<T> // render-thread object

namespace Jupiter::Assets {
namespace fs = std::filesystem;

class AssetManager {
public:
  using RenderTask = std::function<void()>;

  AssetManager() noexcept = default;
  ~AssetManager() noexcept = default;

  AssetManager(const AssetManager &) = delete;
  AssetManager &operator=(const AssetManager &) = delete;
  AssetManager(AssetManager &&) = delete;
  AssetManager &operator=(AssetManager &&) = delete;

  static AssetManager &GetInstance() noexcept {
    static AssetManager instance;
    return instance;
  }

  // WARNING:
  // MUST INITIALIZE FROM THE RENDER THREAD!
  void Init();

  // WARNING:
  // Must be called by the render thread
  // This destroys all GPU-backed assets while the graphics context is still
  // alive
  void Shutdown() noexcept;

  // WARNING:
  // Must be called by the render thread
  void ProcessRenderTasks();

  // WARNING:
  // Must be called by the render thread
  void PollHotReload();

  bool IsRenderThread() const noexcept {
    return std::this_thread::get_id() == m_RenderThreadID;
  }

  template <class T>
  static AssetHandle<T> Get(const fs::path &path, LoadOptions opts = {}) {
    return GetInstance().GetAssetHelper<T>(path, std::move(opts));
  }

  template <class T> static Ref<T> Resolve(AssetHandle<T> handle) {
    return GetInstance().ResolveHelper<T>(handle);
  }

  // Runtime / generated asset
  //
  // The factory itself runs on the render thread
  //
  // Example:
  //
  // AssetManager::Create<Mesh>("quad", [&](){
  //    return Mesh::Create(...);
  // });

  template <class T, class Factory>
  static AssetHandle<T> Create(std::string_view name, Factory &&factory) {
    return GetInstance().CreateHelper<T>(name, std::forward<Factory>(factory));
  }

private:
  struct AssetRecord {
    AssetType type;
    std::string name;

    fs::path path;
    fs::file_time_type last_write_time;
    LoadOptions load_options;

    u32 pool_idx, pool_gen;

    bool runtime_created;
  };

  struct PendingReload {
    AssetType type;
    AssetID id;
    fs::path path;
  };

  AssetPools m_AssetPools;
  std::unordered_map<AssetID, AssetRecord> m_Registry;

  fs::path m_AssetsDir;

  // Registry synchronization is primarily for the watcher and render thread
  mutable std::shared_mutex m_Mutex;

  // Watcher thread -> render thread
  // m_PendingMutex guards only m_PendingReloads
  std::mutex m_PendingMutex;
  std::vector<PendingReload> m_PendingReloads;

  // Game thread -> render thread
  // m_RenderTasksMutex guards only m_RenderTasks

  std::mutex m_RenderTasksMutex;
  std::vector<RenderTask> m_RenderTasks;

  // Lets the watcher thread nudge the render thread the moment it queues a
  // hot-reload, instead of waiting for the next poll to pick it up
  std::function<void()> m_RendererWakeup;

  std::thread::id m_RenderThreadID{};

  std::jthread m_WatcherThread;
  std::chrono::milliseconds m_PollInterval{300};

  void WatcherThreadMain(std::stop_token stopToken);

  void QueueRenderTask(RenderTask task) {
    std::scoped_lock lock(m_RenderTasksMutex);
    m_RenderTasks.push_back(std::move(task));
  }

  template <class T>
  AssetHandle<T> GetAssetHelper(const fs::path &relativePath,
                                const LoadOptions &opts) {
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

  template <class T> Ref<T> ResolveHelper(const AssetHandle<T> &handle) const {
    std::shared_lock lock(m_Mutex);
    const auto *value = m_AssetPools.GetPool<T>().Resolve(handle);
    return value ? *value : Ref<T>{};
  }

  // Reserve+register happen synchronously so concurrent Create() calls for
  // the same name still dedupe correctly regardless of which thread they
  // come from. factory() itself is queued and only ever runs later, on the
  // render thread, out of ProcessRenderTasks().
  template <class T, class Factory>
  AssetHandle<T> CreateHelper(std::string_view name, Factory &&factory) {
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

  template <class T> void ReloadHelper(const PendingReload &job) {
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

    auto reloaded = AssetTraits<T>::LoadFromFile(job.path, record.load_options);
    if (!reloaded) {
      LOG_ERROR("AssetManager: hot reload FAILED for {} '{}'",
                AssetTraits<T>::Str, job.path.string());

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

    LOG_INFO("AssetManager: reloaded {} '{}'", AssetTraits<T>::Str,
             record.name);
  }
};
} // namespace Jupiter::Assets
