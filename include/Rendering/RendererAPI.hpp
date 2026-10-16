#pragma once

#include "Rendering/Backend/IRenderer.hpp"
#include "Rendering/Backend/RenderScheduler.hpp"
#include "Rendering/Backend/IResourceFactory.hpp"
#include "Rendering/Backend/DrawCommand.hpp"
#include "Rendering/GraphicsAPI.hpp"

#include <functional>
#include <atomic>
#include <future>
#include <type_traits>
#include <utility>

namespace Jupiter::Rendering {

class RendererAPI {
public:
  // ===================================
  // Game thread API.
  // ===================================
  static bool BeginFrame(const SceneData &sceneData);
  static bool SubmitDrawCommand(RHI::DrawCommand command);

  // Runs on the render thread after scene commands and before presentation.
  static bool SubmitOverlay(std::function<void()> overlay);
  static bool EndFrame();

  // Submit a complete snapshot after all simulation workers have joined.
  static bool CommitFrame(Frame frame);
  // Arbitrary render-thread work. Captures must own their CPU data and must
  // not retain mutable game-thread state.
  static bool SubmitRenderTask(RenderTask task);

  // The future becomes ready after render-thread CPU execution. If the task
  // is rejected or cancelled, it reports broken_promise; task exceptions are
  // delivered through the future.
  template <class Fn> static auto SubmitRenderTaskAsync(Fn &&function)
      -> std::pair<bool, std::future<std::invoke_result_t<Fn &, RenderExecutionContext &>>> {
    using Result = std::invoke_result_t<Fn &, RenderExecutionContext &>;
    std::packaged_task<Result(RenderExecutionContext &)> packaged(std::forward<Fn>(function));
    auto future = packaged.get_future();
    const bool accepted =
        SubmitRenderTask([task = std::move(packaged)](RenderExecutionContext &context) mutable { task(context); });
    return {accepted, std::move(future)};
  }

  // ===================================
  // Render thread API.
  // ===================================
  static void Init(Core::Window &window);
  static bool RenderNextFrame();
  static void CancelPending();
  static void Shutdown();
  static bool SetVSync(RHI::VSyncMode mode);

  // ===================================
  // Thread-independent API.
  // ===================================
  static GraphicsAPI GetAPI() { return s_RendererAPI.load(std::memory_order_acquire); }
  static void RequestStop();
  static bool IsStopping();

private:
  static std::atomic<GraphicsAPI> s_RendererAPI;
  static Scope<RHI::IResourceFactory> s_ResourceFactory;
  static Scope<RHI::IRenderer> s_RendererImplementation;
  static RenderScheduler s_RenderScheduler;
};

} // namespace Jupiter::Rendering
