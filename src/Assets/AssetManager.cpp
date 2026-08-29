#include "AssetManager.hpp"
#include "Types.hpp"

#include "Core/Logging.hpp"
#include "Core/Assert.hpp"

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
      ReloadHelper<Rendering::IShader>(job);
      break;
    case Texture:
      ReloadHelper<Rendering::ITexture>(job);
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
} // namespace Jupiter::Assets
