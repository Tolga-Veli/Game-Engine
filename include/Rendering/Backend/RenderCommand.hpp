#pragma once

#include <functional>

namespace Jupiter::Rendering::RHI {
struct RenderCommand {
  RenderCommand() noexcept = default;
  ~RenderCommand() noexcept = default;

  explicit RenderCommand(std::function<void()> _cmd) noexcept : cmd(std::move(_cmd)) {}

  RenderCommand(const RenderCommand &) noexcept = default;
  RenderCommand &operator=(const RenderCommand &) noexcept = default;
  RenderCommand(RenderCommand &&) noexcept = default;
  RenderCommand &operator=(RenderCommand &&) noexcept = default;

  std::function<void()> cmd;
};
} // namespace Jupiter::Rendering::RHI
