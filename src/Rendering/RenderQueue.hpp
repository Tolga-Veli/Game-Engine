#pragma once

#include "Rendering/Backend/IRenderer.hpp"
#include "RenderCommand.hpp"
#include "SceneData.hpp"

#include <condition_variable>

namespace Jupiter::Rendering {

struct Frame {
  SceneData sceneData;
  std::vector<RenderCommand> commands;

  void Clear() { commands.clear(); }
};

class RenderQueue {
public:
  RenderQueue();

  void BeginFrame(const SceneData &sceneData);
  void Submit(const RenderCommand &command);
  void EndFrame();

  void Stop();

  Frame *WaitForFrame();

  void ReleaseFrame(Frame *frame);

private:
  Frame m_Storage[2];

  Frame *m_Build{}, *m_Ready{}, *m_Free{};

  std::mutex m_Mutex;
  std::condition_variable m_ReadyCV, m_FreeCV;
  bool m_Stopping{false};
};
} // namespace Jupiter::Rendering
