#include "Renderer/Backend/IVertexBuffer.hpp"
#include "Renderer/OpenGL/VertexBuffer.hpp"
#include "Renderer/Renderer.hpp"

namespace Jupiter::Renderer {
Ref<IVertexBuffer> IVertexBuffer::Create(u32 size, BufferUsage usage) {
  switch (Renderer::GetAPI()) {
  case RendererAPI::OpenGL:
    return std::make_shared<GLVertexBuffer>(size, usage);
  default:
    return nullptr;
  }
}

Ref<IVertexBuffer> IVertexBuffer::Create(const void *data, u32 size,
                                         BufferUsage usage) {
  switch (Renderer::GetAPI()) {
  case RendererAPI::OpenGL:
    return std::make_shared<GLVertexBuffer>(data, size, usage);
  default:
    return nullptr;
  }
}
} // namespace Jupiter::Renderer
