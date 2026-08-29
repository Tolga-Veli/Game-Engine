#pragma once

#include "RenderQueue.hpp"

namespace Jupiter::Rendering {
RenderQueue::RenderQueue() : m_Build(&m_Storage[0]), m_Free(&m_Storage[1]) {}

void RenderQueue::BeginFrame(const SceneData &sceneData) {
  std::scoped_lock lock(m_Mutex);

  if (m_Stopping)
    return;

  m_Build->sceneData = sceneData;
  m_Build->Clear();
}

void RenderQueue::Submit(const RenderCommand &cmd) {
  m_Build->commands.push_back(cmd);
}

void RenderQueue::EndFrame() {
  Frame *justBuilt = m_Build;

  std::unique_lock lock(m_Mutex);
  m_Ready = justBuilt;
  m_ReadyCV.notify_one();

  // Wait for the render thread to hand back the other buffer we start
  // building the next frame into it.

  m_FreeCV.wait(lock, [this] { return m_Free != nullptr || m_Stopping; });
  if (m_Free) {
    m_Build = m_Free;
    m_Free = nullptr;
  }
}

void RenderQueue::Stop() {
  {
    std::scoped_lock lock(m_Mutex);
    m_Stopping = true;
  }
  m_ReadyCV.notify_all();
  m_FreeCV.notify_all();
}

Frame *RenderQueue::WaitForFrame() {
  std::unique_lock lock(m_Mutex);
  m_ReadyCV.wait(lock, [this] { return m_Ready != nullptr || m_Stopping; });

  if (!m_Ready)
    return nullptr;

  Frame *frame = m_Ready;
  m_Ready = nullptr;
  return frame;
}

void RenderQueue::ReleaseFrame(Frame *frame) {
  {
    std::scoped_lock lock(m_Mutex);
    m_Free = frame;
  }
  m_FreeCV.notify_one();
}
} // namespace Jupiter::Rendering
