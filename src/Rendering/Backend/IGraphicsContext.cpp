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
    CORE_VERIFY(false, "not implemented yet");
  }
  return nullptr;
}

} // namespace Jupiter::Rendering::RHI
