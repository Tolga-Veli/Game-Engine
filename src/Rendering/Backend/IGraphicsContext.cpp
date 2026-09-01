#include "Rendering/Backend/IGraphicsContext.hpp"
#include "Core/Assert.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/GraphicsContext.hpp"

namespace Jupiter::Rendering::RHI {
Scope<IGraphicsContext> IGraphicsContext::Create(GLFWwindow *handle) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return CreateScope<OpenGL::GraphicsContext>(handle);
  default:
    UNREACHABLE("Unsupported graphics api");
    return {};
  }
}

} // namespace Jupiter::Rendering::RHI
