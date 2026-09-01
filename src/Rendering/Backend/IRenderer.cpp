#include "Rendering/Backend/IRenderer.hpp"

#include "Core/Assert.hpp"
#include "Rendering/OpenGL/Renderer.hpp"
#include "Rendering/Renderer.hpp"

namespace Jupiter::Rendering::RHI {

Scope<IRenderer> IRenderer::Create() {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return CreateScope<OpenGL::Renderer>();
  default:
    UNREACHABLE("Unsupported graphics api");
    return {};
  }
}
} // namespace Jupiter::Rendering::RHI
