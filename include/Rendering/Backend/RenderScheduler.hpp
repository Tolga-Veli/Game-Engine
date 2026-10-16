#pragma once

#include "Rendering/Backend/FrameQueue.hpp"
#include "Rendering/Backend/RendererCommandQueue.hpp"

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <array>

namespace Jupiter::Rendering {
// The two payload queues share this ordering lock and wakeup. A committed
// frame is indivisible; arbitrary tasks run between frames.
class RenderScheduler {
public:
  struct Work {
    Frame frame;
    RenderTask task;
    bool isFrame{};
  };

  RenderScheduler() = default;
  RenderScheduler(const RenderScheduler &) = delete;
  RenderScheduler &operator=(const RenderScheduler &) = delete;

  void Reset();
  void Stop() noexcept;
  bool IsStopping();
  void CancelPending(); // render thread, before graphics context destruction

  bool BeginFrame(const SceneData &sceneData);
  // These builder calls belong to the thread that began the frame. Workers
  // prepare independent lists for CommitFrame instead.
  bool Submit(RHI::DrawCommand command);
  bool SubmitOverlay(std::function<void()> overlay);
  bool EndFrame();
  bool CommitFrame(Frame frame);
  bool Enqueue(RenderTask task);
  bool WaitNext(Work &work);
  void CompleteFrame() noexcept;
  void CompleteFrame(Frame frame) noexcept;

private:
  static constexpr std::size_t k_MaxFramesInFlight = 2;
  static constexpr std::size_t k_MaxPendingTasks = 4096;
  FrameQueue m_Frames;
  RendererCommandQueue m_Commands;
  Frame m_Build;
  std::array<Frame, k_MaxFramesInFlight + 1> m_Recycled;
  std::size_t m_RecycledCount{};
  bool m_Building{}, m_Publishing{}, m_Stopping{true};
  std::thread::id m_FrameProducer{};
  std::uint64_t m_NextTicket{};
  std::uint64_t m_Epoch{};
  std::mutex m_Mutex;
  std::condition_variable m_WorkCV, m_FrameCV;
};
} // namespace Jupiter::Rendering
