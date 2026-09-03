#include "Rendering/Renderer.hpp"

namespace Jupiter::Rendering {

GraphicsAPI Renderer::s_RendererAPI = GraphicsAPI::OpenGL;
Scope<RHI::IRenderer> Renderer::s_RendererImplementation;
RenderQueue Renderer::s_RenderQueue{};

void Renderer::Init() {
  s_RenderQueue.Reset();
  s_RendererImplementation = RHI::IRenderer::Create();
  s_RendererImplementation->Init();
}

void Renderer::Shutdown() {
  // Frames retain shared references to GPU resources. Release them while the
  // graphics context is still current instead of at static destruction time.
  s_RenderQueue.Reset();

  if (!s_RendererImplementation)
    return;

  s_RendererImplementation->Shutdown();
  s_RendererImplementation.reset();
}

bool Renderer::RenderNextFrame() {
  Frame *frame = s_RenderQueue.WaitForFrame();
  if (!frame)
    return false;

  s_RendererImplementation->BeginFrame(frame->sceneData);

  for (const auto &cmd : frame->commands)
    s_RendererImplementation->Submit(cmd);

  s_RendererImplementation->EndFrame();
  s_RenderQueue.ReleaseFrame(frame);
  return true;
}

void Renderer::RequestStop() { s_RenderQueue.Stop(); }

void Renderer::Wake() { s_RenderQueue.Wake(); }

void Renderer::BeginFrame(const SceneData &sceneData) {
  s_RenderQueue.BeginFrame(sceneData);
}

void Renderer::Submit(RHI::RenderCommand cmd) {
  if (!cmd.mesh || !cmd.material)
    return;

  s_RenderQueue.Submit(std::move(cmd));
}

void Renderer::EndFrame() { s_RenderQueue.EndFrame(); }

} // namespace Jupiter::Rendering
