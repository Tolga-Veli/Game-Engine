#include "IVertexArray.hpp"

#include "Rendering/OpenGL/VertexArray.hpp"
#include "Rendering/Renderer.hpp"

#include "Core/Assert.hpp"

namespace Jupiter::Rendering {
Ref<IVertexArray> IVertexArray::Create() {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_shared<OpenGL::VertexArray>();
  default:
    CORE_ASSERT(false, "IVertexBuffer::Create() Renderer API not implemented");
    return nullptr;
  }
}
} // namespace Jupiter::Rendering
