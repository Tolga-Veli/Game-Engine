#include "Renderer/Backend/IVertexArray.hpp"
#include "Renderer/OpenGL/VertexArray.hpp"
#include "Renderer/Renderer.hpp"

namespace Jupiter::Renderer {
Ref<IVertexArray> IVertexArray::Create() {
  switch (Renderer::GetAPI()) {
  case RendererAPI::OpenGL:
    return std::make_shared<GLVertexArray>();
  default:
    return nullptr;
  }
}
} // namespace Jupiter::Renderer
