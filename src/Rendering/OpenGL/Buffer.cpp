#include "Rendering/OpenGL/Buffer.hpp"
#include "Rendering/Backend/IBuffer.hpp"

#include <cstring>

namespace Jupiter::Rendering::OpenGL {
Buffer::Buffer(const RHI::BufferDesc &desc) noexcept : m_Desc(desc) {
  CORE_ASSERT(m_Desc.size > 0, "Buffer must have a non-zero size");

  glCreateBuffers(1, &m_RendererID);
  CORE_ASSERT(m_RendererID != 0, "Failed to create OpenGL buffer");

  const auto usage = JupiterToOpenGLBufferUsage(m_Desc.usage);

  /*
   * Static:
   *   immutable storage, initialized once.
   *
   * Dynamic:
   *   immutable storage, but GL_DYNAMIC_STORAGE_BIT allows
   *   glNamedBufferSubData().
   *
   * Stream:
   *   persistently mapped coherent storage.
   *
   * Passing initialData directly here is particularly important for
   * Static buffers: storage with flags == 0 cannot subsequently be
   * modified with glNamedBufferSubData().
   */

  glNamedBufferStorage(m_RendererID, m_Desc.size, m_Desc.initData, usage);

  if (m_Desc.usage == RHI::BufferUsage::Stream) {
    constexpr GLbitfield mapFlags =
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    m_MappedData =
        glMapNamedBufferRange(m_RendererID, 0, m_Desc.size, mapFlags);

    CORE_ASSERT(m_MappedData != nullptr, "Failed to map stream buffer");
  }
}

Buffer::~Buffer() noexcept { Destroy(); }

Buffer::Buffer(Buffer &&other) noexcept
    : m_RendererID(std::exchange(other.m_RendererID, 0)), m_Desc(other.m_Desc),
      m_MappedData(std::exchange(other.m_MappedData, nullptr)) {
  other.m_Desc = {};
}

Buffer &Buffer::operator=(Buffer &&other) noexcept {
  if (this == &other)
    return *this;

  Destroy();

  m_RendererID = std::exchange(other.m_RendererID, 0);
  m_Desc = other.m_Desc;
  m_MappedData = std::exchange(other.m_MappedData, nullptr);

  other.m_Desc = {};

  return *this;
}

void Buffer::Destroy() noexcept {
  if (m_RendererID == 0)
    return;

  /*
   * Only Stream buffers are persistently mapped.
   */
  if (m_MappedData != nullptr) {
    glUnmapNamedBuffer(m_RendererID);
    m_MappedData = nullptr;
  }

  glDeleteBuffers(1, &m_RendererID);
  m_RendererID = 0;
}

void Buffer::Bind() const noexcept {
  CORE_ASSERT(m_RendererID != 0, "Attempted to bind invalid OpenGL buffer");

  glBindBuffer(JupiterToOpenGLBufferType(m_Desc.type), m_RendererID);
}

void Buffer::Unbind() const noexcept {
  glBindBuffer(JupiterToOpenGLBufferType(m_Desc.type), 0);
}

void Buffer::SetData(const void *data, u32 size, u32 offset) noexcept {
  CORE_ASSERT(m_RendererID != 0,
              "OpenGL::Buffer::SetData() called on invalid buffer");

  CORE_ASSERT(data != nullptr, "OpenGL::Buffer::SetData() received null data");

  CORE_ASSERT(offset <= m_Desc.size,
              "OpenGL::Buffer::SetData() offset is out of bounds");

  CORE_ASSERT(size <= m_Desc.size - offset,
              "OpenGL::Buffer::SetData() range is out of bounds");

  switch (m_Desc.usage) {
  case RHI::BufferUsage::Static:
    /*
     * Static buffers were created with immutable storage and no
     * GL_DYNAMIC_STORAGE_BIT. They must never be modified.
     *
     * Upload initial contents through BufferDesc::initialData instead.
     */
    CORE_ASSERT(false, "Cannot call SetData() on a static buffer; "
                       "provide initialData in BufferDesc");

    return;

  case RHI::BufferUsage::Dynamic:
    /*
     * GL_DYNAMIC_STORAGE_BIT was supplied during allocation,
     * therefore glNamedBufferSubData is legal.
     */
    glNamedBufferSubData(m_RendererID, static_cast<GLintptr>(offset),
                         static_cast<GLsizeiptr>(size), data);

    return;

  case RHI::BufferUsage::Stream:
    /*
     * Stream buffers stay persistently mapped. No GL upload command
     * is necessary; CPU writes directly into the mapped storage.
     */
    CORE_ASSERT(m_MappedData != nullptr, "Stream buffer is not mapped");

    std::memcpy(static_cast<std::byte *>(m_MappedData) + offset, data, size);

    return;
  }

  CORE_ASSERT(false, "Unknown RHI::BufferUsage");
}

} // namespace Jupiter::Rendering::OpenGL
