#include "Rendering/Backend/RenderThread.hpp"
#include "Rendering/Renderer.hpp"

#include <future>

namespace Jupiter::Rendering {

void RenderThread::Start(RenderThreadHooks hooks) {
  m_Stopping = false;

  std::promise<void> initPromise;
  auto initFuture = initPromise.get_future();

  m_Thread = std::thread([this, hooks = std::move(hooks),
                          promise = std::move(initPromise)]() mutable {
    hooks.Init();
    promise.set_value();

    while (!m_Stopping) {
      hooks.OnFrame();
      Renderer::RenderNextFrame();
    }

    hooks.Shutdown();
  });

  // Block the caller until setup has actually run
  initFuture.wait();
}

void RenderThread::Stop() {
  m_Stopping = true;

  Renderer::RequestStop();

  if (m_Thread.joinable())
    m_Thread.join();
}

} // namespace Jupiter::Rendering
