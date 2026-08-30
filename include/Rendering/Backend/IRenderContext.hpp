#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Rendering::RHI {
class IRenderContext {
public:
  virtual ~IRenderContext() = default;

  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  virtual void Present() = 0;
};
} // namespace Jupiter::Rendering::RHI
