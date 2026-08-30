#include "Rendering/OpenGL/Buffer.hpp"

namespace Jupiter::Rendering::OpenGL {
Buffer::Buffer(const RHI::BufferDesc &desc) noexcept : m_Desc(desc) {
  CORE_ASSERT(m_Desc.size > 0, "Buffer must have a non-zero size");

  glCreateBuffers(1, &m_RendererID);
  glNamedBufferStorage(m_RendererID, m_Desc.size, nullptr,
                       JupiterToOpenGLBufferStorageFlags(m_Desc.usage));
}

void Buffer::SetData(const void *data, u32 size, u32 offset) noexcept {
  CORE_ASSERT(offset <= m_Desc.size, "Offset exceeds buffer size");
  CORE_ASSERT(size <= m_Desc.size - offset, "Range exceeds buffer size");

  glNamedBufferSubData(m_RendererID, offset, size, data);
}

} // namespace Jupiter::Rendering::OpenGL
