#pragma once

#include "Core/Utils.hpp"

#include "RenderCommand.hpp"
#include "SceneData.hpp"

namespace Jupiter::Rendering::RHI {

class IRenderer {
public:
  virtual ~IRenderer() = default;

  static Scope<IRenderer> Create();

  virtual void Init() = 0;
  virtual void Shutdown() = 0;

  virtual void BeginFrame(const SceneData &sceneData) = 0;
  virtual void EndFrame() = 0;

  virtual void Submit(const RenderCommand &cmd) = 0;
};
}; // namespace Jupiter::Rendering::RHI
