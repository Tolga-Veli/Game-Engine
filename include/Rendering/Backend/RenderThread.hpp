#pragma once

#include <functional>
#include <thread>

namespace Jupiter::Rendering {

struct RenderThreadHooks {
  // Runs once, on the render thread, before any frames are processed. This is
  // where the context gets made and every resource (shaders,
  // textures, meshes, materials) gets created - Start() blocks the calling
  // thread until this has finished, so callers can treat render-thread startup
  // as a normal synchronous step
  std::function<void()> Init;

  // Shutdown runs once, on the render thread, after the last frame has been
  // rendererd
  std::function<void()> Shutdown;

  // Runs every frame, on the render thread, right after a frame has been
  // rendererd.
  std::function<void()> OnFrame;
};

class RenderThread {
public:
  // Blocks until hooks.Init has finished running on the render thread
  void Start(RenderThreadHooks hooks);

  // Signals the render thread to finish up and joins it.
  void Stop();

private:
  std::thread m_Thread;
  std::atomic<bool> m_Stopping{false};

  void Run(RenderThreadHooks hooks);
};
} // namespace Jupiter::Rendering
