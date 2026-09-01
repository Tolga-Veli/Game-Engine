#pragma once

#include "Core/Utils.hpp"

#include "Types.hpp"
#include "AssetPools.hpp"
#include "AssetTraits.hpp"
#include "Core/Assert.hpp"

#include "Traits/TextureTraits.hpp"
#include "Traits/ShaderTraits.hpp"
#include "Traits/MaterialTraits.hpp"
#include "Traits/MeshTraits.hpp"
#include "Traits/FontTraits.hpp"

#include <filesystem>
#include <chrono>
#include <functional>
#include <thread>
#include <mutex>
#include <shared_mutex>

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

  [[nodiscard]]
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
  // Executes tasks on the render thread queue by other threads
  void ProcessRenderTasks();

  // WARNING:
  // Must be called by the render thread
  // Processes filesystem changes detected by the watcher thread
  void PollHotReload();

  void SetRendererWakeup(std::function<void()> wakeup) {
    m_RendererWakeup = std::move(wakeup);
  }

  [[nodiscard]] bool IsRenderThread() const noexcept {
    return std::this_thread::get_id() == m_RenderThreadID;
  }

  template <Asset T>
  static AssetHandle<T>
  Get(const fs::path &path,
      const typename AssetTraits<T>::LoadOptions &opts = {}) {
    return GetInstance().GetAsset<T>(path, opts);
  }

  template <Asset T> static Ref<T> Resolve(AssetHandle<T> handle) {
    return GetInstance().ResolveAsset<T>(handle);
  }

  // Register runtime created assets synchronously
  //
  // The factory is NOT executed immediately. It is queued to be executed later
  // on the render thread by ProcessRenderTasks().

  template <Asset T, class Factory>
  static AssetHandle<T> Create(std::string_view name, Factory &&factory) {
    return GetInstance().CreateAsset<T>(name, std::forward<Factory>(factory));
  }

private:
  struct AssetRecord {
    AssetType type;

    std::string name;
    fs::path path;
    fs::file_time_type lastWriteTime;
    LoadOptions loadOptions;

    u64 poolIndex, poolGeneration;
    bool runtimeCreated;
  };

  struct PendingReload {
    AssetType type;
    AssetID id;
    fs::path path;
  };

  // -----------------------------------------------------
  //  State
  // ------------------------------------------------------
  AssetPools m_AssetPools;
  std::unordered_map<AssetID, AssetRecord> m_Registry;

  fs::path m_AssetsDir;

  // Registry synchronization for the watcher and render thread
  mutable std::shared_mutex m_RegistryMutex;

  // -----------------------------------------------------
  //  Watcher -> Render thread
  // ------------------------------------------------------
  std::mutex m_PendingReloadMutex;
  std::vector<PendingReload> m_PendingReloads;

  // -----------------------------------------------------
  //  Game thread -> Render thread
  // ------------------------------------------------------
  std::mutex m_RenderTasksMutex;
  std::vector<RenderTask> m_RenderTasks;

  // Lets the watcher thread nudge the render thread the moment it queues a
  // hot-reload, instead of waiting for the next poll to pick it up
  std::function<void()> m_RendererWakeup;

  std::thread::id m_RenderThreadID{};

  // -----------------------------------------------------
  //  File watcher
  // ------------------------------------------------------
  std::jthread m_WatcherThread;
  std::chrono::milliseconds m_PollInterval{300};

private:
  void WatcherThreadMain(std::stop_token stopToken);

  void QueueRenderTask(RenderTask task) {
    {
      std::scoped_lock lock(m_RenderTasksMutex);
      m_RenderTasks.push_back(std::move(task));
    }

    if (m_RendererWakeup)
      m_RendererWakeup();
  }

  template <Asset T>
  AssetHandle<T> GetAsset(const fs::path &relativePath,
                          const typename AssetTraits<T>::LoadOptions &opts) {
    fs::path path = (m_AssetsDir / relativePath).lexically_normal();
    std::string name = relativePath.generic_string();
    AssetID id = AssetTraits<T>::MakeID(path, opts);

    {
      std::shared_lock lock(m_RegistryMutex);
      if (auto it = m_Registry.find(id); it != m_Registry.end())
        return AssetHandle<T>{it->second.poolIndex, it->second.poolGeneration};
    }

    std::error_code ec;
    const auto writeTime = fs::last_write_time(path, ec);
    if (ec)
      LOG_WARN("AssetManager: couldn't determine modification time for '{}'",
               path.string());

    AssetHandle<T> handle;
    {
      std::unique_lock lock(m_RegistryMutex);

      // might have already been created
      if (auto it = m_Registry.find(id); it != m_Registry.end())
        return AssetHandle<T>{it->second.poolIndex, it->second.poolGeneration};

      handle = m_AssetPools.GetPool<T>().Reserve();
      m_Registry.emplace(id, AssetRecord{.type = AssetTraits<T>::type,
                                         .name = name,
                                         .path = path,
                                         .lastWriteTime = writeTime,
                                         .loadOptions = opts,
                                         .poolIndex = handle.GetID(),
                                         .poolGeneration = handle.GetGen(),
                                         .runtimeCreated = false});
    }

    QueueRenderTask([this, id, handle, path, name, opts]() {
      auto value = AssetTraits<T>::LoadFromFile(path, opts);

      std::unique_lock lock(m_RegistryMutex);
      auto &pool = m_AssetPools.GetPool<T>();

      if (!pool.IsValid(handle))
        return;

      if (!value) {
        LOG_ERROR("AssetManager: failed to load {} '{}'",
                  to_string(AssetTraits<T>::type), path.string());

        pool.Release(handle);
        m_Registry.erase(id);
        return;
      }

      pool.Replace(handle, CreateAssetFromImpl<T>(std::move(value)));
      LOG_INFO("AssetManager: loaded {} '{}'", to_string(AssetTraits<T>::type),
               name);
    });

    return handle;
  }

  template <Asset T> Ref<T> ResolveAsset(AssetHandle<T> handle) const {
    std::shared_lock lock(m_RegistryMutex);
    return m_AssetPools.GetPool<T>().Resolve(handle);
  }

  // Reserve+register happen synchronously so concurrent Create() calls for
  // the same name still dedupe correctly regardless of which thread they
  // come from. factory() itself is queued and only ever runs later, on the
  // render thread, out of ProcessRenderTasks().
  template <Asset T, class Factory>
  AssetHandle<T> CreateAsset(std::string_view name, Factory &&factory) {
    AssetID id = HashRuntimeAssetName(name, AssetTraits<T>::type);
    {
      std::shared_lock lock(m_RegistryMutex);
      if (auto it = m_Registry.find(id); it != m_Registry.end())
        return AssetHandle<T>{it->second.poolIndex, it->second.poolGeneration};
    }

    std::string nameStr{name};
    AssetHandle<T> handle;
    {
      std::unique_lock lock(m_RegistryMutex);

      // someone may have beaten us to it between the two locks above
      if (auto it = m_Registry.find(id); it != m_Registry.end())
        return AssetHandle<T>{it->second.poolIndex, it->second.poolGeneration};

      handle = m_AssetPools.GetPool<T>().Reserve();
      m_Registry.emplace(id, AssetRecord{.type = AssetTraits<T>::type,
                                         .name = nameStr,
                                         .path = {},
                                         .lastWriteTime = {},
                                         .loadOptions = {},
                                         .poolIndex = handle.GetID(),
                                         .poolGeneration = handle.GetGen(),
                                         .runtimeCreated = true});
    }

    QueueRenderTask([this, id, handle, nameStr,
                     factory = std::forward<Factory>(factory)]() mutable {
      CORE_ASSERT(IsRenderThread(),
                  "Runtime asset factory must run on render thread");

      // NOTE:
      // GPU creation happens here, on the render thread, whenever
      // ProcessRenderTasks() next picks this task up.
      auto value = factory();

      std::unique_lock lock(m_RegistryMutex);
      auto &pool = m_AssetPools.GetPool<T>();

      if (!pool.IsValid(handle))
        return;

      if (!value) {
        LOG_ERROR("AssetManager: failed to create runtime {} '{}'",
                  to_string(AssetTraits<T>::type), nameStr);

        pool.Release(handle);
        m_Registry.erase(id);
        return;
      }

      pool.Replace(handle, CreateAssetFromImpl<T>(std::move(value)));

      LOG_INFO("AssetManager: created runtime {} '{}'",
               to_string(AssetTraits<T>::type), nameStr);
    });

    return handle;
  }

  template <Asset T> void ReloadAsset(const PendingReload &job) {
    CORE_ASSERT(IsRenderThread(), "Asset reload must run on render thread");

    AssetRecord record;
    {
      std::shared_lock lock(m_RegistryMutex);
      auto it = m_Registry.find(job.id);
      if (it == m_Registry.end())
        return;

      record = it->second;
    }

    if (record.runtimeCreated || record.path.empty())
      return;

    const auto &opts =
        std::get<typename AssetTraits<T>::LoadOptions>(record.loadOptions);

    auto impl = AssetTraits<T>::LoadFromFile(job.path, opts);

    if (!impl) {
      LOG_ERROR("AssetManager: reload FAILED for {} '{}'",
                to_string(AssetTraits<T>::type), job.path.string());
      return;
    }
    auto reloaded = CreateAssetFromImpl<T>(std::move(impl));

    {
      std::unique_lock lock(m_RegistryMutex);
      auto it = m_Registry.find(job.id);
      if (it == m_Registry.end())
        return;

      AssetHandle<T> handle{it->second.poolIndex, it->second.poolGeneration};
      if (!m_AssetPools.GetPool<T>().Replace(handle, std::move(reloaded)))
        return;
    }

    LOG_INFO("AssetManager: reloaded {} '{}'", to_string(AssetTraits<T>::type),
             record.name);
  }

  template <Asset T>
  static Ref<T> CreateAssetFromImpl(Scope<typename AssetTraits<T>::Impl> impl) {
    return Ref<T>(new T(std::move(impl)));
    // std::make_shared doesn't work since every Asset's constructor is private
  }
};
} // namespace Jupiter::Assets
