#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <memory>

namespace Jupiter::Rendering {
namespace RHI {
class IRenderer;
class IResourceFactory;
} // namespace RHI

struct RenderExecutionContext {
  RHI::IRenderer &renderer;
  RHI::IResourceFactory &resources;
};

using RenderTask = std::move_only_function<void(RenderExecutionContext &)>;

class RendererCommandQueue {
  friend class RenderScheduler;

  struct Entry {
    std::uint64_t ticket{};
    std::unique_ptr<RenderTask> task;
  };

  std::deque<Entry> ready;
};
} // namespace Jupiter::Rendering
