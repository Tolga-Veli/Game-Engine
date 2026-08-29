#include "IGraphicsContext.hpp"
#include "Core/Assert.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/GraphicsContext.hpp"

namespace Jupiter::Rendering {
Scope<IGraphicsContext> IGraphicsContext::Create(GLFWwindow *handle) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_unique<OpenGL::GraphicsContext>(handle);
  default:
    CORE_VERIFY(false, "not implemented yet");
  }
  return nullptr;
}

} // namespace Jupiter::Rendering
