#include "IIndexBuffer.hpp"
#include "Core/Utils.hpp"
#include "Renderer/OpenGL/IndexBuffer.hpp"
#include "Renderer/Renderer.hpp"

namespace Jupiter::Renderer {

Ref<IIndexBuffer> IIndexBuffer::Create(const u32 *indices, u32 count,
                                       BufferUsage usage) {
  switch (Renderer::GetAPI()) {
  case RendererAPI::OpenGL:
    return std::make_shared<GLIndexBuffer>(indices, count, usage);
  default:
    return nullptr;
  }
}
} // namespace Jupiter::Renderer
