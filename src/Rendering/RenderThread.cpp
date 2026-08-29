#include "RenderThread.hpp"
#include "Renderer.hpp"

#include <future>

namespace Jupiter::Rendering {

void RenderThread::Start(RenderThreadHooks hooks) {
  std::promise<void> initPromise;
  auto initFuture = initPromise.get_future();

  m_Thread = std::thread(
      [hooks = std::move(hooks), promise = std::move(initPromise)]() mutable {
        hooks.Init();
        promise.set_value();

        while (Renderer::RenderNextFrame())
          hooks.OnFrame();

        hooks.Shutdown();
      });

  // Block the caller until setup has actually run
  initFuture.wait();
}

void RenderThread::Stop() {
  Renderer::RequestStop();

  if (m_Thread.joinable())
    m_Thread.join();
}

} // namespace Jupiter::Rendering
