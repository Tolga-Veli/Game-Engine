#pragma once

#include "Rendering/Backend/IRenderCommandQueue.hpp"
#include "Rendering/Backend/RenderCommand.hpp"
#include "Data/SPSC_Queue.hpp"

namespace Jupiter::Rendering::OpenGL {
class RenderCommandQueue final : public RHI::IRenderCommandQueue {
public:
  bool try_push(RHI::RenderCommand cmd) override { return m_Queue.try_push(std::move(cmd)); }
  bool try_pop(RHI::RenderCommand &cmd) override { return m_Queue.try_pop(cmd); }

private:
  static constexpr std::size_t k_Size = 1024;
  Data::SPSC_Queue<RHI::RenderCommand, k_Size> m_Queue;
};
} // namespace Jupiter::Rendering::OpenGL
