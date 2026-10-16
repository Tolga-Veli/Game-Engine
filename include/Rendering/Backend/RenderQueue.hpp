#pragma once

#include "Rendering/Backend/DrawCommand.hpp"
#include "Rendering/Backend/SceneData.hpp"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

namespace Jupiter::Rendering {

struct Frame {
  SceneData sceneData{};
  std::vector<RHI::DrawCommand> commands;
  std::vector<std::function<void()>> overlays;

  void Clear() {
    commands.clear();
    overlays.clear();
  }
};

class RenderQueue {
public:
  RenderQueue();

  RenderQueue(const RenderQueue &) = delete;
  RenderQueue &operator=(const RenderQueue &) = delete;

  void BeginFrame(const SceneData &sceneData);
  void Submit(RHI::DrawCommand command);
  void SubmitOverlay(std::function<void()> overlay);
  void EndFrame();

  void Stop();
  void Wake();
  void Reset();

  Frame *WaitForFrame();
  Frame *TryGetFrame();
  void ReleaseFrame(Frame *frame);

private:
  Frame m_Storage[2];
  Frame *m_Build{}, *m_Ready{}, *m_Free{};

  // Frame returned by WaitForFrame owned by the consumer until ReleaseFrame is called
  Frame *m_Rendering{};

  std::mutex m_Mutex;
  std::condition_variable m_ReadyCV, m_FreeCV;
  bool m_Stopping{false}, m_Wakeup{false};
};
} // namespace Jupiter::Rendering
