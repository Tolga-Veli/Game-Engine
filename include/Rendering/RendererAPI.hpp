#pragma once

#include "Rendering/Backend/IRenderer.hpp"
#include "Rendering/Backend/RenderQueue.hpp"
#include "Rendering/Backend/IResourceFactory.hpp"
#include "Rendering/Backend/IRenderCommandQueue.hpp"
#include "Rendering/Backend/DrawCommand.hpp"

#include <functional>

namespace Jupiter::Rendering {
enum class GraphicsAPI { None = 0, OpenGL, Vulkan, Metal, DX11, DX12, Count };

class RendererAPI {
public:
  // ===================================
  // Game thread API.
  // ===================================
  static void BeginFrame(const SceneData &sceneData);
  static void SubmitDrawCommand(RHI::DrawCommand command);
  // Runs on the render thread after scene commands and before presentation.
  static void SubmitOverlay(std::function<void()> overlay);
  static void EndFrame(); // Blocks until the render thread finishes consuming
                          // the frame - bounds the
                          // game thread to being at most one frame ahead.

  // ===================================
  // Render thread API.
  // ===================================
  static void Init(GLFWwindow *window);
  static bool RenderNextFrame();
  static void Shutdown();
  static void SetVSync(RHI::VSyncMode mode) { s_RendererImplementation->SetVSync(mode); }

  // ===================================
  // Thread-independent API.
  // ===================================
  static GraphicsAPI GetAPI() { return s_RendererAPI; }
  static void RequestStop();
  static void Wake();

  static Scope<RHI::IResourceFactory> &GetResourceFactory() { return s_ResourceFactory; }

private:
  static GraphicsAPI s_RendererAPI;
  static Scope<RHI::IResourceFactory> s_ResourceFactory;
  static Scope<RHI::IRenderer> s_RendererImplementation;
  static RenderQueue s_RenderQueue;
  static Scope<RHI::IRenderCommandQueue> m_RenderCommandQueue;
};

} // namespace Jupiter::Rendering
