#include "IVertexBuffer.hpp"

#include "Rendering/OpenGL/VertexBuffer.hpp"
#include "Rendering/Renderer.hpp"

#include "Core/Assert.hpp"

namespace Jupiter::Rendering {
Ref<IVertexBuffer> IVertexBuffer::Create(u32 size, BufferUsage usage) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_shared<OpenGL::VertexBuffer>(size, usage);
  default:
    CORE_ASSERT(false, "IVertexBuffer::Create() Renderer API not implemented");
    return nullptr;
  }
}

Ref<IVertexBuffer> IVertexBuffer::Create(const void *data, u32 size,
                                         BufferUsage usage) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_shared<OpenGL::VertexBuffer>(data, size, usage);
  default:
    CORE_ASSERT(false, "IVertexBuffer::Create() Renderer API not implemented");
    return nullptr;
  }
}
} // namespace Jupiter::Rendering
