#include "IIndexBuffer.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/IndexBuffer.hpp"

namespace Jupiter::Rendering {
Ref<IIndexBuffer> IIndexBuffer::Create(const u32 *indices, u32 count,
                                       BufferUsage usage) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_shared<OpenGL::IndexBuffer>(indices, count, usage);
  default:
    return nullptr;
  }
}
} // namespace Jupiter::Rendering
