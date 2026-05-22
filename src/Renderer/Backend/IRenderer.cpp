#include "IRenderer.hpp"
#include "Renderer/OpenGL/OpenGLRenderer.hpp"
#include "Renderer/Renderer.hpp"

namespace Jupiter::Renderer {

Scope<IRenderer> IRenderer::Create() {
  switch (Renderer::GetAPI()) {
  case RendererAPI::OpenGL:
    return std::make_unique<OpenGLRenderer>();
  default:
    return nullptr;
  }
}
} // namespace Jupiter::Renderer
