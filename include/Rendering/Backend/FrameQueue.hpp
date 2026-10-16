#pragma once

#include "Rendering/Backend/DrawCommand.hpp"
#include "Rendering/Backend/SceneData.hpp"

#include <cstdint>
#include <cstddef>
#include <deque>
#include <functional>
#include <vector>

namespace Jupiter::Rendering {
struct Frame {
  SceneData sceneData{};
  std::vector<RHI::DrawCommand> commands;
  std::vector<std::function<void()>> overlays;
};

class FrameQueue {
  friend class RenderScheduler;
  struct Entry {
    std::uint64_t ticket{};
    Frame frame;
  };
  std::deque<Entry> ready;
  std::size_t outstanding{};
};
} // namespace Jupiter::Rendering
