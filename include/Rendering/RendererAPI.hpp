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
