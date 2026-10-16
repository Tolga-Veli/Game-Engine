#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <mutex>
#include <exception>

namespace Jupiter::Rendering {

struct RenderThreadHooks {
  std::function<bool()> Init;     // runs on the render thread; false = abort startup
  std::function<void()> Shutdown; // runs on the render thread, GL context still alive
};

class RenderThread {
public:
  RenderThread() = default;
  ~RenderThread() noexcept { Stop(); }

  RenderThread(const RenderThread &) = delete;
  RenderThread &operator=(const RenderThread &) = delete;
  RenderThread(RenderThread &&) = delete;
  RenderThread &operator=(RenderThread &&) = delete;

  [[nodiscard]] bool Start(RenderThreadHooks hooks);

  void Stop() noexcept;

  [[nodiscard]] bool HasFailed() const noexcept { return m_Failed.load(std::memory_order_acquire); }

  // Game thread: call once per frame so a render-thread exception surfaces instead of hanging.
  void RethrowIfFailed();

private:
  std::thread m_Thread;
  std::atomic<bool> m_Stopping{false}, m_Failed{false};

  std::mutex m_FailureMutex;
  std::exception_ptr m_Failure;

  void RecordFailure(std::exception_ptr failure) noexcept;
};
} // namespace Jupiter::Rendering
