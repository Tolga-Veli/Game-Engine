#include "Rendering/Backend/RenderScheduler.hpp"

#include <stdexcept>
#include <utility>

namespace Jupiter::Rendering {
void RenderScheduler::Reset() {
  std::scoped_lock lock(m_Mutex);
  if (m_Frames.outstanding || m_Building || m_Publishing || !m_Frames.ready.empty() || !m_Commands.ready.empty())
    throw std::logic_error("RenderScheduler::Reset with outstanding work");

  m_NextTicket = 0;
  ++m_Epoch;
  m_Stopping = false;
  m_FrameProducer = {};
}

void RenderScheduler::Stop() noexcept {
  {
    std::scoped_lock lock(m_Mutex);
    m_Stopping = true;
  }
  m_WorkCV.notify_all();
  m_FrameCV.notify_all();
}

bool RenderScheduler::IsStopping() {
  std::scoped_lock lock(m_Mutex);
  return m_Stopping;
}

void RenderScheduler::CancelPending() {
  std::deque<FrameQueue::Entry> frames;
  std::deque<RendererCommandQueue::Entry> commands;
  Frame build;
  {
    std::scoped_lock lock(m_Mutex);
    frames.swap(m_Frames.ready);
    commands.swap(m_Commands.ready);
    build = std::move(m_Build);
    m_Building = false;
    m_Frames.outstanding -= frames.size();
  }
  // Captures die on the render thread while its context is current.
  m_FrameCV.notify_all();
}

bool RenderScheduler::BeginFrame(const SceneData &sceneData) {
  std::scoped_lock lock(m_Mutex);
  if (m_Stopping || m_Building || m_Publishing)
    return false;

  m_FrameProducer = std::this_thread::get_id();
  if (m_RecycledCount)
    m_Build = std::move(m_Recycled[--m_RecycledCount]);
  m_Build.sceneData = sceneData;
  m_Building = true;
  return true;
}

bool RenderScheduler::Submit(RHI::DrawCommand command) {
  std::scoped_lock lock(m_Mutex);
  if (m_Stopping || !m_Building || m_FrameProducer != std::this_thread::get_id())
    return false;

  m_Build.commands.push_back(std::move(command));
  return true;
}

bool RenderScheduler::SubmitOverlay(std::function<void()> overlay) {
  if (!overlay)
    return false;

  std::scoped_lock lock(m_Mutex);
  if (m_Stopping || !m_Building || m_FrameProducer != std::this_thread::get_id())
    return false;

  m_Build.overlays.push_back(std::move(overlay));
  return true;
}

bool RenderScheduler::EndFrame() {
  Frame sealed;
  std::unique_lock lock(m_Mutex);
  if (m_Stopping)
    return false;

  if (!m_Building || m_FrameProducer != std::this_thread::get_id())
    throw std::logic_error("EndFrame must be called by the frame producer");
  // Seal before waiting for a free slot. Worker submissions after this point
  // cannot change the frame that will be published.
  sealed = std::move(m_Build);
  m_Building = false;
  m_Publishing = true;
  const auto epoch = m_Epoch;
  m_FrameCV.wait(lock, [this, epoch] {
    return m_Stopping || epoch != m_Epoch || m_Frames.outstanding < k_MaxFramesInFlight;
  });

  if (m_Stopping || epoch != m_Epoch) {
    m_Publishing = false;
    return false;
  }
  try {
    auto &entry = m_Frames.ready.emplace_back();
    entry.ticket = m_NextTicket++;
    entry.frame = std::move(sealed);
  } catch (...) {
    m_Publishing = false;
    throw;
  }
  ++m_Frames.outstanding;
  m_Publishing = false;
  lock.unlock();
  m_WorkCV.notify_one();
  return true;
}

bool RenderScheduler::CommitFrame(Frame frame) {
  std::unique_lock lock(m_Mutex);
  const auto epoch = m_Epoch;
  m_FrameCV.wait(lock, [this, epoch] {
    return m_Stopping || epoch != m_Epoch || m_Frames.outstanding < k_MaxFramesInFlight;
  });
  if (m_Stopping || epoch != m_Epoch)
    return false;

  auto &entry = m_Frames.ready.emplace_back();
  entry.ticket = m_NextTicket++;
  entry.frame = std::move(frame);
  ++m_Frames.outstanding;
  lock.unlock();
  m_WorkCV.notify_one();
  return true;
}

bool RenderScheduler::Enqueue(RenderTask task) {
  if (!task)
    return false;
  // Moving a small callable can run its capture's move constructor/destructor.
  // Keep that user code outside the ordering lock as well.
  auto owned = std::make_unique<RenderTask>(std::move(task));
  {
    std::scoped_lock lock(m_Mutex);
    if (m_Stopping || m_Commands.ready.size() >= k_MaxPendingTasks)
      return false;
    auto &entry = m_Commands.ready.emplace_back();
    entry.ticket = m_NextTicket++;
    entry.task = std::move(owned);
  }
  m_WorkCV.notify_one();
  return true;
}

bool RenderScheduler::WaitNext(Work &work) {
  // User captures can submit work from their destructors. Never destroy them
  // while holding the scheduler lock.
  work = {};
  std::unique_ptr<RenderTask> task;
  std::unique_lock lock(m_Mutex);
  m_WorkCV.wait(lock, [this] { return m_Stopping || !m_Frames.ready.empty() || !m_Commands.ready.empty(); });
  if (m_Stopping)
    return false;

  const bool takeFrame = !m_Frames.ready.empty() &&
                         (m_Commands.ready.empty() || m_Frames.ready.front().ticket < m_Commands.ready.front().ticket);
  if (takeFrame) {
    work.frame = std::move(m_Frames.ready.front().frame);
    m_Frames.ready.pop_front();
    work.isFrame = true;
  } else {
    task = std::move(m_Commands.ready.front().task);
    m_Commands.ready.pop_front();
  }
  lock.unlock();
  if (task)
    work.task = std::move(*task);
  return true;
}

void RenderScheduler::CompleteFrame() noexcept {
  {
    std::scoped_lock lock(m_Mutex);
    --m_Frames.outstanding;
  }
  m_FrameCV.notify_all();
}

void RenderScheduler::CompleteFrame(Frame frame) noexcept {
  frame.commands.clear();
  frame.overlays.clear();
  {
    std::scoped_lock lock(m_Mutex);
    if (m_RecycledCount < m_Recycled.size())
      m_Recycled[m_RecycledCount++] = std::move(frame);
    --m_Frames.outstanding;
  }
  m_FrameCV.notify_all();
}
} // namespace Jupiter::Rendering
