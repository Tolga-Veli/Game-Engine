#pragma once

#include "RenderCommand.hpp"

namespace Jupiter::Rendering::RHI {
class IRenderCommandQueue {
public:
  virtual ~IRenderCommandQueue() = default;

  virtual bool try_push(RenderCommand cmd) = 0;
  virtual bool try_pop(RenderCommand &cmd) = 0;
};
} // namespace Jupiter::Rendering::RHI
