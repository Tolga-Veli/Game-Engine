#include "Assets/AssetManager.hpp"

#include "Core/Logging.hpp"
#include "Core/Assert.hpp"

#include <mutex>
#include <thread>

namespace Jupiter::Assets {
void AssetManager::Init() {
  m_RenderThreadID = std::this_thread::get_id();

  const fs::path base = fs::current_path();
  const fs::path assetsDir = base / "assets";
  if (!fs::exists(assetsDir) || !fs::is_directory(assetsDir)) {
    LOG_ERROR("AssetManager: FAILED to find assets folder at '{}'",
              assetsDir.string());

    m_RenderThreadID = {};
    return;
  }

  m_AssetsDir = assetsDir.lexically_normal();

  LOG_INFO("AssetManager: Root established at {}", m_AssetsDir.string());

  m_WatcherThread =
      std::jthread([this](std::stop_token st) { WatcherThreadMain(st); });

  // If anything might have been queued up by Create(Runtime) before
  // initializing
  Assets::AssetManager::GetInstance().ProcessRenderTasks();
}

void AssetManager::ProcessRenderTasks() {
  CORE_ASSERT(IsRenderThread(), "AssetManager: ProcessRenderTasks() must run "
                                "on the render thread");

  std::vector<RenderTask> tasks;
  {
    std::scoped_lock lock(m_RenderTasksMutex);
    tasks.swap(m_RenderTasks);
  }

  for (auto &task : tasks)
    if (task)
      task();
}

// WARNING:
// Pool destruction happens here while the graphics context is still alive
void AssetManager::Shutdown() noexcept {
  CORE_ASSERT(IsRenderThread(), "AssetManager: Shutdown() must run "
                                "on the render thread");

  m_WatcherThread.request_stop();

  if (m_WatcherThread.joinable())
    m_WatcherThread.join();

  ProcessRenderTasks();
  {
    std::unique_lock lock(m_RegistryMutex);

    // Asset pools own the actual GPU-backed resources.
    // They MUST be destroyed before the graphics context disappears.
    m_Registry.clear();
    m_AssetPools.Clear();
  }

  {
    std::scoped_lock lock(m_RenderTasksMutex);
    m_RenderTasks.clear();
  }

  {
    std::scoped_lock lock(m_PendingReloadMutex);
    m_PendingReloads.clear();
  }

  m_RendererWakeup = {};
  m_RenderThreadID = {};

  LOG_INFO("AssetManager: Resources cleared");
}

void AssetManager::PollHotReload() {
  CORE_ASSERT(IsRenderThread(), "AssetManager: PollHotReload() must run "
                                "on the render thread");

  std::vector<PendingReload> jobs;
  {
    std::scoped_lock lock(m_PendingReloadMutex);

    if (m_PendingReloads.empty())
      return;

    jobs.swap(m_PendingReloads);
  }

  for (const auto &job : jobs) {
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

      snapshot.reserve(m_Registry.size());
      for (const auto &[id, record] : m_Registry) {
        if (record.runtimeCreated || record.path.empty())
          continue;

        snapshot.push_back(
            {id, record.type, record.path, record.lastWriteTime});
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
        m_PendingReloads.push_back({tracked.type, tracked.id, tracked.path});
      }

      if (m_RendererWakeup)
        m_RendererWakeup();
    }
    std::this_thread::sleep_for(m_PollInterval);
  }
}

} // namespace Jupiter::Assets
