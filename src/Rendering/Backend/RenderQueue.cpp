#include "Rendering/Backend/RenderQueue.hpp"

namespace Jupiter::Rendering {
RenderQueue::RenderQueue() : m_Build(&m_Storage[0]), m_Free(&m_Storage[1]) {}

void RenderQueue::BeginFrame(const SceneData &sceneData) {
  std::scoped_lock lock(m_Mutex);

  if (m_Stopping || !m_Build)
    return;

  m_Build->Clear();
  m_Build->sceneData = sceneData;
}

void RenderQueue::Submit(RHI::DrawCommand cmd) {
  std::scoped_lock lock(m_Mutex);

  if (m_Stopping || !m_Build)
    return;

  m_Build->commands.push_back(std::move(cmd));
}

void RenderQueue::SubmitOverlay(std::function<void()> overlay) {
  if (!overlay)
    return;

  std::scoped_lock lock(m_Mutex);

  if (m_Stopping || !m_Build)
    return;

  m_Build->overlays.push_back(std::move(overlay));
}

void RenderQueue::EndFrame() {
  std::unique_lock lock(m_Mutex);

  if (m_Stopping || !m_Build)
    return;

  m_ReadyCV.wait(lock, [this] { return m_Stopping || m_Ready == nullptr; });

  if (m_Stopping)
    return;

  Frame *finished = m_Build;
  m_Build = nullptr;
  m_Ready = finished;

  lock.unlock();
  m_ReadyCV.notify_one();
  lock.lock();

  m_FreeCV.wait(lock, [this] { return m_Stopping || m_Free != nullptr; });

  if (m_Stopping)
    return;

  m_Build = std::exchange(m_Free, nullptr);
}

void RenderQueue::Stop() {
  {
    std::scoped_lock lock(m_Mutex);
    m_Stopping = true;
    m_Wakeup = true;

    // Discard the ready frame. Do not clear it here: that could destroy resource references on the producer thread
    m_Ready = nullptr;
    m_Build = nullptr;
  }

  m_ReadyCV.notify_all();
  m_FreeCV.notify_all();
}

void RenderQueue::Wake() {
  {
    std::scoped_lock lock(m_Mutex);

    if (m_Stopping)
      return;

    m_Wakeup = true;
  }

  m_ReadyCV.notify_one();
}

void RenderQueue::Reset() {
  std::scoped_lock lock(m_Mutex);

  if (m_Rendering != nullptr)
    throw std::logic_error("RenderQueue: Reset() called with a frame in flight");

  for (auto &frame : m_Storage)
    frame.Clear();

  m_Build = &m_Storage[0];
  m_Ready = nullptr;
  m_Free = &m_Storage[1];
  m_Stopping = false;
  m_Wakeup = false;
}

Frame *RenderQueue::WaitForFrame() {
  {
    std::unique_lock lock(m_Mutex);

    m_ReadyCV.wait(lock, [this] { return m_Ready != nullptr || m_Stopping || m_Wakeup; });

    m_Wakeup = false;

    if (m_Stopping || !m_Ready)
      return nullptr;

    if (m_Rendering != nullptr)
      throw std::logic_error("RenderQueue already has a frame in flight");

    m_Rendering = std::exchange(m_Ready, nullptr);
  }

  m_ReadyCV.notify_all();
  return m_Rendering;
}

Frame *RenderQueue::TryGetFrame() {
  std::scoped_lock lock(m_Mutex);

  if (m_Stopping || !m_Ready || m_Rendering)
    return nullptr;

  m_Rendering = std::exchange(m_Ready, nullptr);
  m_ReadyCV.notify_all();

  return m_Rendering;
}

void RenderQueue::ReleaseFrame(Frame *frame) {
  if (!frame)
    return;

  {
    std::scoped_lock lock(m_Mutex);

    if (frame != m_Rendering)
      throw std::logic_error("RenderQueue::ReleaseFrame: invalid frame");

    m_Rendering = nullptr;

    if (!m_Stopping)
      m_Free = frame;
    else
      m_Free = nullptr;
  }

  m_FreeCV.notify_all();
  m_ReadyCV.notify_all();
}

} // namespace Jupiter::Rendering
