#pragma once

#include "Rendering/RenderCommand.hpp"

namespace Jupiter::Rendering {
class ICommandBuffer {
public:
  virtual ~ICommandBuffer() = default;

  virtual void Begin() = 0;
  virtual void End() = 0;

  virtual void Draw(const RenderCommand &command) = 0;

private:
};
} // namespace Jupiter::Rendering
