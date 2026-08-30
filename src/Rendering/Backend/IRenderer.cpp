#include "Rendering/Backend/IRenderer.hpp"

#include "Rendering/OpenGL/Renderer.hpp"
#include "Rendering/Renderer.hpp"

namespace Jupiter::Rendering::RHI {

Scope<IRenderer> IRenderer::Create() {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_unique<OpenGL::Renderer>();
  default:
    return nullptr;
  }
}
} // namespace Jupiter::Rendering::RHI
