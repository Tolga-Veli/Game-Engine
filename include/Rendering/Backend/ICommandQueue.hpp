#pragma once

#include "Core/Utils.hpp"
#include "ICommandBuffer.hpp"

namespace Jupiter::Rendering::RHI {
class ICommandQueue {
public:
  virtual ~ICommandQueue() = default;

  virtual void Submit(const Ref<ICommandBuffer> &cmdBuffer) = 0;

  virtual void WaitIdle() = 0;

private:
};
} // namespace Jupiter::Rendering::RHI
