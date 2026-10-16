#include "Rendering/Backend/RenderThread.hpp"
#include "Rendering/RendererAPI.hpp"

#include <exception>
#include <future>
#include <stdexcept>
#include <utility>

namespace Jupiter::Rendering {

void RenderThread::RecordFailure(std::exception_ptr failure) noexcept {
  {
    std::scoped_lock lock(m_FailureMutex);
    if (!m_Failure)
      m_Failure = std::move(failure);
  }

  m_Failed.store(true, std::memory_order_release);
}

bool RenderThread::Start(RenderThreadHooks hooks) {
  if (m_Thread.joinable())
    throw std::logic_error("RenderThread::Start");

  if (!hooks.Init)
    throw std::invalid_argument("RenderThread::Start() requires hooks.Init");

  m_Stopping.store(false, std::memory_order_release);
  m_Failed.store(false, std::memory_order_release);

  {
    std::scoped_lock lock(m_FailureMutex);
    m_Failure = nullptr;
  }

  std::promise<bool> initPromise;
  auto initFuture = initPromise.get_future();

  m_Thread = std::thread([this, hooks = std::move(hooks), promise = std::move(initPromise)]() mutable {
    bool initialized = false, started = false;

    try {
      initialized = hooks.Init();
      promise.set_value(initialized);
      started = true;

      if (!initialized)
        return;

      while (!m_Stopping.load(std::memory_order_acquire)) {
        if (hooks.OnFrame)
          hooks.OnFrame();

        if (m_Stopping.load(std::memory_order_acquire))
          break;

        RendererAPI::RenderNextFrame();
      }
    } catch (...) {
      const auto failure = std::current_exception();
      RecordFailure(failure);

      if (!started) {
        try {
          promise.set_exception(failure);
          started = true;
        } catch (...) {}
      }

      try {
        RendererAPI::RequestStop();
      } catch (...) { RecordFailure(std::current_exception()); }

      if (initialized)
        m_Stopping.wait(false, std::memory_order_acquire);
    }

    if (initialized && hooks.Shutdown)
      try {
        hooks.Shutdown();
      } catch (...) { RecordFailure(std::current_exception()); }
  });

  try {
    const bool initialized = initFuture.get();

    if (!initialized) {
      Stop();
      return false;
    }
    return true;
  } catch (...) {
    Stop();
    throw;
  }
}

void RenderThread::Stop() noexcept {
  m_Stopping.store(true, std::memory_order_release);
  m_Stopping.notify_all();

  try {
    RendererAPI::RequestStop();
  } catch (...) { RecordFailure(std::current_exception()); }

  if (m_Thread.joinable()) {
    // Never detach: the thread uses this object and its hooks
    if (m_Thread.get_id() == std::this_thread::get_id())
      std::terminate();

    m_Thread.join();
  }
}

void RenderThread::RethrowIfFailed() {
  std::exception_ptr failure;
  {
    std::scoped_lock lock(m_FailureMutex);
    failure = m_Failure;
  }

  if (failure)
    std::rethrow_exception(failure);
}

} // namespace Jupiter::Rendering
