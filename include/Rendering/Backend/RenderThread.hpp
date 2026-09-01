#pragma once

#include <atomic>
#include <functional>
#include <thread>

namespace Jupiter::Rendering {

struct RenderThreadHooks {
  // Runs once, on the render thread, before any frames are processed. This is
  // where the context gets made and every resource (shaders,
  // textures, meshes, materials) gets created - Start() blocks the calling
  // thread until this has finished, so callers can treat render-thread startup
  // as a normal synchronous step
  std::function<bool()> Init;

  // Shutdown runs once, on the render thread, after the last frame has been
  // rendered.
  std::function<void()> Shutdown;

  // Runs on the render thread before it waits for the next frame. Use this for
  // queued render-thread work such as asset creation and hot reloads.
  std::function<void()> OnFrame;

  // Runs after a frame has been rendered. Presentation belongs here so the
  // newly rendered back buffer is the one that gets displayed.
  std::function<void()> Present;
};

class RenderThread {
public:
  // Blocks until hooks.Init has finished running on the render thread
  [[nodiscard]] bool Start(RenderThreadHooks hooks);

  ~RenderThread() { Stop(); }

  // Signals the render thread to finish up and joins it.
  void Stop();

private:
  std::thread m_Thread;
  std::atomic<bool> m_Stopping{false};
};
} // namespace Jupiter::Rendering
