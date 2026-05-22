#include "Renderer.hpp"

namespace Jupiter::Renderer {

RendererAPI Renderer::s_RendererAPI = RendererAPI::OpenGL;
Scope<IRenderer> Renderer::s_RendererImplementation = IRenderer::Create();

void Renderer::Init() { s_RendererImplementation->Init(); }

void Renderer::Shutdown() { s_RendererImplementation->Shutdown(); }

void Renderer::BeginScene(const SceneData &sceneData) {
  s_RendererImplementation->BeginScene(sceneData);
}

void Renderer::EndScene() { s_RendererImplementation->EndScene(); }

void Renderer::Submit(const Ref<Mesh> &mesh, const Ref<Material> &material,
                      const glm::mat4 &transform) {
  material->Bind();
  material->ApplyParameters();
  s_RendererImplementation->Submit(RenderCommand{mesh, material, transform});
}

void Renderer::Clear(const glm::vec4 &color) {
  s_RendererImplementation->Clear(color);
}

void Renderer::OnWindowResize(u32 width, u32 height) {
  s_RendererImplementation->OnWindowResize(width, height);
}
} // namespace Jupiter::Renderer
