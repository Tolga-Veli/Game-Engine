#pragma once

#include "Core/Utils.hpp"

#include <glm/glm.hpp>

namespace Jupiter::Rendering::RHI {

struct RenderCommand;
struct SceneData;

class IRenderer {
public:
  virtual ~IRenderer() = default;

  virtual void Init() = 0;
  virtual void Shutdown() = 0;

  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  virtual void Submit(const RenderCommand &cmd) = 0;

  static Scope<IRenderer> Create();
};
}; // namespace Jupiter::Rendering::RHI
