#include "Rendering/Backend/RenderThread.hpp"
#include "Rendering/Renderer.hpp"

#include <exception>
#include <future>

namespace Jupiter::Rendering {

bool RenderThread::Start(RenderThreadHooks hooks) {
  m_Stopping = false;

  std::promise<bool> initPromise;
  auto initFuture = initPromise.get_future();

  m_Thread = std::thread([this, hooks = std::move(hooks),
                          promise = std::move(initPromise)]() mutable {
    bool initialized = false;
    try {
      initialized = hooks.Init();
      promise.set_value(initialized);
    } catch (...) {
      promise.set_exception(std::current_exception());
      return;
    }

    if (!initialized)
      return;

    while (!m_Stopping) {
      hooks.OnFrame();
      if (Renderer::RenderNextFrame() && hooks.Present)
        hooks.Present();
    }

    hooks.Shutdown();
  });

  // Block the caller until setup has actually run
  try {
    const bool initialized = initFuture.get();
    if (!initialized && m_Thread.joinable())
      m_Thread.join();
    return initialized;
  } catch (...) {
    if (m_Thread.joinable())
      m_Thread.join();
    throw;
  }
}

void RenderThread::Stop() {
  if (!m_Thread.joinable())
    return;

  m_Stopping = true;

  Renderer::RequestStop();

  m_Thread.join();
}

} // namespace Jupiter::Rendering
