#include "Assets/AssetManager.hpp"

#include "Core/Logging.hpp"
#include "Core/Assert.hpp"

#include <mutex>
#include <thread>

namespace Jupiter::Assets {
void AssetManager::Init() {
  std::unique_lock registryLock(m_RegistryMutex);
  if (m_Active)
    throw std::logic_error("AssetManager: already initialized");

  const fs::path base = fs::current_path();
  fs::path assetsDir = base / "assets";
  if (!fs::exists(assetsDir) || !fs::is_directory(assetsDir)) {
    const fs::path buildAssets = base / "bin" / "assets";
    if (fs::is_directory(buildAssets))
      assetsDir = buildAssets;
  }

  if (!fs::exists(assetsDir) || !fs::is_directory(assetsDir)) {
    LOG_ERROR("AssetManager: FAILED to find assets folder at '{}'", assetsDir.string());
    throw std::runtime_error("AssetManager: assets directory not found");
  }

  m_AssetsDir = assetsDir.lexically_normal();
  m_RenderThreadID = std::this_thread::get_id();
  m_Active = true;
  {
    std::scoped_lock loadLock(m_LoadMutex);
    m_LoadStopping = false;
  }

  LOG_INFO("AssetManager: Root established at {}", m_AssetsDir.string());

  registryLock.unlock();
  try {
    m_LoadThread = std::jthread([this](std::stop_token st) { LoadThreadMain(st); });
    m_WatcherThread = std::jthread([this](std::stop_token st) { WatcherThreadMain(st); });
  } catch (...) {
    Shutdown();
    throw;
  }
}

// WARNING:
// Pool destruction happens here while the graphics context is still alive
void AssetManager::Shutdown() noexcept {
  {
    std::unique_lock lock(m_RegistryMutex);
    CORE_VERIFY(IsRenderThread(), "AssetManager: Shutdown() must run on the render thread");
    m_Active = false;
  }

  m_WatcherThread.request_stop();

  if (m_WatcherThread.joinable())
    m_WatcherThread.join();

  {
    std::scoped_lock lock(m_LoadMutex);
    m_LoadStopping = true;
  }
  m_LoadThread.request_stop();
  m_LoadCV.notify_all();
  if (m_LoadThread.joinable())
    m_LoadThread.join();
  m_LoadJobs.clear();

  {
    std::unique_lock lock(m_RegistryMutex);

    // Asset pools own the actual GPU-backed resources.
    // They MUST be destroyed before the graphics context disappears.
    m_Registry.clear();
    m_FileAssets.clear();
    m_AssetPools.Clear();
    m_RenderThreadID = {};
  }

  {
    std::scoped_lock lock(m_PendingReloadMutex);
    m_PendingReloads.clear();
    m_ReloadScheduled = false;
  }

  LOG_INFO("AssetManager: Resources cleared");
}

void AssetManager::PollHotReload(Rendering::RenderExecutionContext &context) {
  (void)context;
  CORE_ASSERT(IsRenderThread(), "AssetManager: PollHotReload() must run "
                                "on the render thread");

  std::unordered_map<AssetID, PendingReload> jobs;
  {
    std::scoped_lock lock(m_PendingReloadMutex);

    jobs.swap(m_PendingReloads);
    m_ReloadScheduled = false;
  }

  for (const auto &[id, job] : jobs) {
    using enum AssetType;

    switch (job.type) {
    case Shader:
      ReloadAsset<Rendering::Shader>(job);
      break;
    case Texture:
      ReloadAsset<Rendering::Texture2D>(job);
      break;
    case Font:
      ReloadAsset<Rendering::Font>(job);
      break;
    case Mesh:
      ReloadAsset<Rendering::Mesh>(job);
      break;
    case Material:
      ReloadAsset<Rendering::Material>(job);
      break;
    case Sampler:
      ReloadAsset<Rendering::Sampler>(job);
      break;
    default:
      UNREACHABLE("Unsupported job reload type");
      break;
    }
  }
}

void AssetManager::WatcherThreadMain(std::stop_token st) {
  struct Tracked {
    AssetID id;
    AssetType type;
    fs::path path;
    fs::file_time_type lastWriteTime;
  };

  while (!st.stop_requested()) {
    std::vector<Tracked> snapshot;
    {
      std::shared_lock lock(m_RegistryMutex);

      snapshot.reserve(m_FileAssets.size());
      for (const auto id : m_FileAssets) {
        const auto &record = m_Registry.at(id);
        if (record.runtimeCreated || record.path.empty() || record.loading)
          continue;

        snapshot.push_back({id, record.type, record.path, record.lastWriteTime});
      }
    }

    for (const auto &tracked : snapshot) {
      if (st.stop_requested())
        break;

      std::error_code ec;
      const auto current = fs::last_write_time(tracked.path, ec);

      if (ec || current == tracked.lastWriteTime)
        continue;

      bool shouldQueue{false};
      {
        std::unique_lock lock(m_RegistryMutex);
        auto it = m_Registry.find(tracked.id);

        if (it == m_Registry.end())
          continue;

        if (it->second.lastWriteTime != current) {
          it->second.lastWriteTime = current;
          shouldQueue = true;
        }
      }

      if (!shouldQueue)
        continue;

      {
        std::scoped_lock pendingLock(m_PendingReloadMutex);
        m_PendingReloads.insert_or_assign(tracked.id, PendingReload{tracked.type, tracked.id, tracked.path});
      }
    }
    {
      std::scoped_lock lock(m_PendingReloadMutex);
      if (!m_PendingReloads.empty() && !m_ReloadScheduled)
        m_ReloadScheduled =
            QueueRenderTask([this](Rendering::RenderExecutionContext &context) { PollHotReload(context); });
    }
    std::unique_lock waitLock(m_WatcherMutex);
    m_WatcherCV.wait_for(waitLock, st, m_PollInterval, [] { return false; });
  }
}

bool AssetManager::QueueLoad(std::move_only_function<void()> job) {
  {
    std::scoped_lock lock(m_LoadMutex);
    if (m_LoadStopping || m_LoadJobs.size() >= k_MaxPendingLoads)
      return false;
    m_LoadJobs.push_back(std::move(job));
  }
  m_LoadCV.notify_one();
  return true;
}

void AssetManager::LoadThreadMain(std::stop_token st) {
  while (!st.stop_requested()) {
    std::move_only_function<void()> job;
    {
      std::unique_lock lock(m_LoadMutex);
      if (!m_LoadCV.wait(lock, st, [this] { return m_LoadStopping || !m_LoadJobs.empty(); }) || m_LoadStopping)
        return;
      job = std::move(m_LoadJobs.front());
      m_LoadJobs.pop_front();
    }
    job();
  }
}

bool AssetManager::QueueRenderTask(RenderTask task) {
  return Rendering::RendererAPI::SubmitRenderTask(std::move(task));
}
} // namespace Jupiter::Assets
