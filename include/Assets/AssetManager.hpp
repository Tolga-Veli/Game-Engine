#pragma once

#include "Core/Utils.hpp"

#include "Types.hpp"
#include "AssetPools.hpp"
#include "AssetTraits.hpp"
#include "Rendering/Backend/RendererCommandQueue.hpp"

// NOLINTBEGIN
#include "Traits/TextureTraits.hpp"
#include "Traits/ShaderTraits.hpp"
#include "Traits/MaterialTraits.hpp"
#include "Traits/MeshTraits.hpp"
#include "Traits/FontTraits.hpp"
#include "Traits/SamplerTraits.hpp"
// NOLINTEND

#include <filesystem>
#include <chrono>
#include <functional>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <deque>
#include <unordered_set>

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
  using RenderTask = Rendering::RenderTask;

  ~AssetManager() noexcept = default;

  AssetManager(const AssetManager &) = delete;
  AssetManager &operator=(const AssetManager &) = delete;
  AssetManager(AssetManager &&) = delete;
  AssetManager &operator=(AssetManager &&) = delete;

  [[nodiscard]] static AssetManager &GetInstance() noexcept {
    static AssetManager instance;
    return instance;
  }

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
  // on the render thread by the renderer command queue.
  template <Asset T> static AssetHandle<T> Create(std::string_view name, typename AssetTraits<T>::AssetData data) {
    return GetInstance().CreateAsset<T>(name, std::move(data));
  }

  // Render-thread only. Use handles and immutable draw data on simulation threads.
  template <Asset T> static Ref<T> Resolve(AssetHandle<T> handle) { return GetInstance().ResolveAsset<T>(handle); }

  template <Asset T> static AssetState GetState(AssetHandle<T> handle) {
    auto &manager = GetInstance();
    std::shared_lock lock(manager.m_RegistryMutex);
    return manager.m_Active ? manager.m_AssetPools.GetPool<T>().GetState(handle) : AssetState::Invalid;
  }

  // Ordered with frames and other render tasks. Existing handles become stale
  // when the render thread releases the slot.
  template <Asset T> static bool Unload(AssetHandle<T> handle);

  // WARNING:
  // Must be called by the render thread
  // Processes filesystem changes detected by the watcher thread
  void PollHotReload(Rendering::RenderExecutionContext &context);

private:
  struct AssetRecord {
    AssetType type;

    std::string name;
    fs::path path;
    fs::file_time_type lastWriteTime;
    LoadOptions loadOptions;

    u64 poolIndex, poolGeneration;
    bool runtimeCreated;
    bool loading{true};
  };

  struct PendingReload {
    AssetType type;
    AssetID id;
    fs::path path;
  };

  static constexpr std::size_t k_MaxPendingLoads = 256;
  std::chrono::milliseconds m_PollInterval{300};

  // -----------------------------------------------------
  //  State
  // ------------------------------------------------------
  AssetPools m_AssetPools;
  std::unordered_map<AssetID, AssetRecord> m_Registry;
  std::unordered_set<AssetID> m_FileAssets;

  fs::path m_AssetsDir;
  bool m_Active{};

  // Registry synchronization for the watcher and render thread
  mutable std::shared_mutex m_RegistryMutex;

  // -----------------------------------------------------
  //  Watcher -> Render thread
  // ------------------------------------------------------
  std::mutex m_PendingReloadMutex;
  std::unordered_map<AssetID, PendingReload> m_PendingReloads;
  bool m_ReloadScheduled{};

  std::thread::id m_RenderThreadID{};

  // -----------------------------------------------------
  //  File watcher
  // ------------------------------------------------------
  std::jthread m_WatcherThread, m_LoadThread;
  std::mutex m_LoadMutex, m_WatcherMutex;

  std::condition_variable_any m_LoadCV, m_WatcherCV;

  std::deque<std::move_only_function<void()>> m_LoadJobs;

  bool m_LoadStopping{true};

private:
  AssetManager() = default;

  void WatcherThreadMain(std::stop_token stopToken);
  void LoadThreadMain(std::stop_token stopToken);
  bool QueueLoad(std::move_only_function<void()> job);

  bool QueueRenderTask(RenderTask task);

  [[nodiscard]] bool IsRenderThread() const noexcept { return std::this_thread::get_id() == m_RenderThreadID; }

  template <Asset T>
  AssetHandle<T> LoadAsset(const fs::path &relativePath, const typename AssetTraits<T>::LoadOptions &opts);

  template <Asset T> AssetHandle<T> CreateAsset(std::string_view name, typename AssetTraits<T>::AssetData data);

  template <Asset T> void ReloadAsset(const PendingReload &job);

  template <Asset T>
  bool QueueAssetLoad(AssetID id, AssetHandle<T> handle, fs::path path, typename AssetTraits<T>::LoadOptions opts);

  template <Asset T> void PublishAsset(AssetID id, AssetHandle<T> handle,
                                       const typename AssetTraits<T>::AssetData &data,
                                       Rendering::RHI::IResourceFactory &factory);

  template <Asset T> void FailAsset(AssetID id, AssetHandle<T> handle);

  template <Asset T> Ref<T> ResolveAsset(AssetHandle<T> handle) const;

  template <Asset T> static Ref<T> CreateAssetFromImpl(Scope<typename AssetTraits<T>::Impl> impl) {
    // std::make_shared doesn't work since every Asset's constructor is private
    return Ref<T>(new T(std::move(impl)));
  }

  template <Asset T> static Scope<typename AssetTraits<T>::Impl>
  CreateAssetImpl(Rendering::RHI::IResourceFactory &factory, const typename AssetTraits<T>::AssetData &data);
};
} // namespace Jupiter::Assets

#include "AssetManager.inl"
