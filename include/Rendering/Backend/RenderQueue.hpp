#pragma once

#include "Rendering/Backend/IRenderer.hpp"
#include "Rendering/Backend/RenderCommand.hpp"
#include "Rendering/Backend/SceneData.hpp"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

namespace Jupiter::Rendering {

struct Frame {
  SceneData sceneData;
  std::vector<RHI::RenderCommand> commands;
  std::vector<std::function<void()>> overlays;

  void Clear() {
    commands.clear();
    overlays.clear();
  }
};

class RenderQueue {
public:
  RenderQueue();

  void BeginFrame(const SceneData &sceneData);
  void Submit(RHI::RenderCommand command);
  void SubmitOverlay(std::function<void()> overlay);
  void EndFrame();

  void Stop();
  void Wake();
  void Reset();

  Frame *WaitForFrame();

  Frame *TryGetFrame() {
    std::scoped_lock lock(m_Mutex);

    if (m_Stopping || !m_Ready)
      return nullptr;

    Frame *frame = m_Ready;
    m_Ready = nullptr;
    return frame;
  }

  void ReleaseFrame(Frame *frame);

private:
  Frame m_Storage[2];
  Frame *m_Build{}, *m_Ready{}, *m_Free{};

  std::mutex m_Mutex;
  std::condition_variable m_ReadyCV, m_FreeCV;
  bool m_Stopping{false}, m_Wakeup{false};
};
} // namespace Jupiter::Rendering
