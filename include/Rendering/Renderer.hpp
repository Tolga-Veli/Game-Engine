#pragma once

#include "Rendering/Backend/IRenderer.hpp"
#include "Rendering/Backend/RenderQueue.hpp"

namespace Jupiter::Rendering {
enum class GraphicsAPI { None = 0, OpenGL, Vulkan, Metal, DX11, DX12, Count };

class Renderer {
public:
  // ===================================
  // Game thread API.
  // ===================================
  static void BeginFrame(const SceneData &sceneData);
  static void Submit(RHI::RenderCommand command);
  static void EndFrame(); // Blocks until the render thread finishes consuming
                          // the frame - bounds the
                          // game thread to being at most one frame ahead.

  // ===================================
  // Render thread API.
  // ===================================
  static void Init();
  static bool RenderNextFrame();
  static void Shutdown();

  // ===================================
  // Thread-independent API.
  // ===================================
  inline static GraphicsAPI GetAPI() { return s_RendererAPI; }
  static void RequestStop();
  static void Wake();

private:
  static GraphicsAPI s_RendererAPI;
  static Scope<RHI::IRenderer> s_RendererImplementation;
  static RenderQueue s_RenderQueue;
};

} // namespace Jupiter::Rendering
