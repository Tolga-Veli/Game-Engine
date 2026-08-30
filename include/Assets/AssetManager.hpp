#pragma once

#include "Core/Utils.hpp"

#include "Types.hpp"
#include "AssetPools.hpp"
#include "AssetTraits.hpp"

#include <filesystem>
#include <chrono>
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

  template <Asset T>
  static AssetHandle<T>
  Get(const fs::path &path,
      const typename AssetTraits<T>::LoadOptions &opts = {}) {
    return GetInstance().GetAssetHelper<T>(path, opts);
  }

  template <Asset T> static Ref<T> Resolve(AssetHandle<T> handle) {
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

  template <Asset T, class Factory>
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

  template <Asset T>
  AssetHandle<T>
  GetAssetHelper(const fs::path &relativePath,
                 const typename AssetTraits<T>::LoadOptions &opts);

  template <Asset T> Ref<T> ResolveHelper(const AssetHandle<T> &handle) const;

  // Reserve+register happen synchronously so concurrent Create() calls for
  // the same name still dedupe correctly regardless of which thread they
  // come from. factory() itself is queued and only ever runs later, on the
  // render thread, out of ProcessRenderTasks().
  template <Asset T, class Factory>
  AssetHandle<T> CreateHelper(std::string_view name, Factory &&factory);

  template <Asset T> void ReloadHelper(const PendingReload &job);

  template <Asset T>
  static Ref<T> CreateAssetFromImpl(Scope<typename AssetTraits<T>::Impl> impl) {
    return CreateRef<T>(std::move(impl));
  }
};
} // namespace Jupiter::Assets
