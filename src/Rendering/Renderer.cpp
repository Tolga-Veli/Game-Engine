#include "Renderer.hpp"
#include "Material.hpp"

namespace Jupiter::Rendering {

GraphicsAPI Renderer::s_RendererAPI = GraphicsAPI::OpenGL;
Scope<IRenderer> Renderer::s_RendererImplementation;
RenderQueue Renderer::s_RenderQueue{};

void Renderer::Init() {
  s_RendererImplementation = IRenderer::Create();
  s_RendererImplementation->Init();
}

void Renderer::Shutdown() {
  if (!s_RendererImplementation)
    return;

  s_RendererImplementation->Shutdown();
  s_RendererImplementation.reset();
}

bool Renderer::RenderNextFrame() {
  Frame *frame = s_RenderQueue.WaitForFrame();

  if (!frame)
    return false;

  s_RendererImplementation->Clear();
  s_RendererImplementation->BeginScene(frame->sceneData);

  for (const auto &cmd : frame->commands) {
    auto shader = cmd.material->Bind();

    if (!shader)
      continue;

    for (const auto &[name, value] : cmd.overrides)
      ApplyUniform(*shader, name, value);

    s_RendererImplementation->Submit(cmd);
  }

  s_RendererImplementation->EndScene();
  s_RenderQueue.ReleaseFrame(frame);
  return true;
}

void Renderer::RequestStop() { s_RenderQueue.Stop(); }

void Renderer::BeginFrame(const SceneData &sceneData) {
  s_RenderQueue.BeginFrame(sceneData);
}

void Renderer::Submit(const RenderCommand &cmd) {
  if (!cmd.mesh || !cmd.material)
    return;

  s_RenderQueue.Submit(cmd);
}

void Renderer::EndFrame() { s_RenderQueue.EndFrame(); }

} // namespace Jupiter::Rendering
