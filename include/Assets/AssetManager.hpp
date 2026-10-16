#pragma once

#include "Core/Utils.hpp"

#include "Types.hpp"
#include "AssetPools.hpp"
#include "AssetTraits.hpp"

// NOLINTBEGIN
#include "Traits/TextureTraits.hpp"
#include "Traits/ShaderTraits.hpp"
#include "Traits/MaterialTraits.hpp"
#include "Traits/MeshTraits.hpp"
#include "Traits/FontTraits.hpp"
// NOLINTEND

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

  ~AssetManager() noexcept = default;

  AssetManager(const AssetManager &) = delete;
  AssetManager &operator=(const AssetManager &) = delete;
  AssetManager(AssetManager &&) = delete;
  AssetManager &operator=(AssetManager &&) = delete;

  [[nodiscard]] static AssetManager &GetInstance() noexcept {
    static AssetManager instance;
    return instance;
  }

  void SetRendererWakeup(std::function<void()> func) { m_RendererWakeup = std::move(func); }

  // WARNING:
  // MUST INITIALIZE FROM THE RENDER THREAD!
  void Init();

  // WARNING:
  // Must be called from the render thread
  // This destroys all GPU-backed assets while the graphics context is still
  // alive. Pending asset tasks are cancelled without running their factories.
  // Callers must stop submitting new asset work before shutdown begins.
  void Shutdown() noexcept;

  // WARNING: The asset is loaded asynchronously
  template <Asset T>
  static AssetHandle<T> Load(const fs::path &path, const typename AssetTraits<T>::LoadOptions &opts = {}) {
    return GetInstance().LoadAsset<T>(path, opts);
  }

  // WARNING: The factory is NOT executed immediately. It is queued to be executed later
  // on the render thread by ProcessRenderTasks().
  template <Asset T> static AssetHandle<T> Create(std::string_view name, typename AssetTraits<T>::AssetData data) {
    return GetInstance().CreateAsset<T>(name, data);
  }

  template <Asset T> static Ref<T> Resolve(AssetHandle<T> handle) { return GetInstance().ResolveAsset<T>(handle); }

  // WARNING:
  // Must be called by the render thread
  // Executes tasks on the render thread queue by other threads
  void ProcessRenderTasks();

  // WARNING:
  // Must be called by the render thread
  // Processes filesystem changes detected by the watcher thread
  void PollHotReload();

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

  std::function<void()> m_RendererWakeup;
  std::thread::id m_RenderThreadID{};

  // -----------------------------------------------------
  //  File watcher
  // ------------------------------------------------------
  std::jthread m_WatcherThread;
  std::chrono::milliseconds m_PollInterval{300};

private:
  AssetManager() = default;

  void WatcherThreadMain(std::stop_token stopToken);

  void QueueRenderTask(RenderTask task);

  [[nodiscard]] bool IsRenderThread() const noexcept { return std::this_thread::get_id() == m_RenderThreadID; }

  template <Asset T>
  AssetHandle<T> LoadAsset(const fs::path &relativePath, const typename AssetTraits<T>::LoadOptions &opts);

  template <Asset T> AssetHandle<T> CreateAsset(std::string_view name, const typename AssetTraits<T>::AssetData &data);

  template <Asset T> void ReloadAsset(const PendingReload &job);

  template <Asset T> Ref<T> ResolveAsset(AssetHandle<T> handle) const;

  template <Asset T> static Ref<T> CreateAssetFromImpl(Scope<typename AssetTraits<T>::Impl> impl) {
    // std::make_shared doesn't work since every Asset's constructor is private
    return Ref<T>(new T(std::move(impl)));
  }

  template <Asset T>
  static Scope<typename AssetTraits<T>::Impl> CreateAssetImpl(typename AssetTraits<T>::AssetData data);
};
} // namespace Jupiter::Assets

#include "AssetManager.inl"
