#include "Rendering/OpenGL/Buffer.hpp"
#include "Rendering/Backend/IBuffer.hpp"

#include <cstring>

namespace Jupiter::Rendering::OpenGL {
Buffer::Buffer(const RHI::BufferDesc &desc, const RHI::BufferData &data) noexcept
    : m_Type(desc.type), m_Usage(desc.usage), m_DataType(desc.dtype) {
  CORE_ASSERT(!data.empty(), "Buffer must have a non-zero size");

  Allocate(data.size(), data.data());
}

Buffer::~Buffer() noexcept { Destroy(); }

Buffer::Buffer(Buffer &&other) noexcept
    : m_RendererID(std::exchange(other.m_RendererID, 0)), m_Type(other.m_Type), m_Usage(other.m_Usage),
      m_DataType(other.m_DataType), m_Size(std::exchange(other.m_Size, 0)),
      m_MappedData(std::exchange(other.m_MappedData, nullptr)) {}

Buffer &Buffer::operator=(Buffer &&other) noexcept {
  if (this == &other)
    return *this;

  Destroy();

  m_RendererID = std::exchange(other.m_RendererID, 0);
  m_Type = other.m_Type;
  m_Usage = other.m_Usage;
  m_DataType = other.m_DataType;
  m_Size = std::exchange(other.m_Size, 0);
  m_MappedData = std::exchange(other.m_MappedData, nullptr);

  return *this;
}

void Buffer::Allocate(std::size_t size, const void *initialData) noexcept {
  glCreateBuffers(1, &m_RendererID);
  glNamedBufferStorage(m_RendererID, size, initialData, JupiterToOpenGLBufferUsage(m_Usage));
  m_Size = size;

  if (m_Usage == RHI::BufferUsage::Stream) {
    constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    m_MappedData = static_cast<std::byte *>(glMapNamedBufferRange(m_RendererID, 0, m_Size, flags));
  }
}

void Buffer::Reserve(std::size_t size) noexcept {
  if (size <= m_Size)
    return;

  u32 newRendererID{};
  glCreateBuffers(1, &newRendererID);

  glNamedBufferStorage(newRendererID, size, nullptr, JupiterToOpenGLBufferUsage(m_Usage));

  const std::size_t sz = std::min(m_Size, size);
  if (sz > 0)
    glCopyNamedBufferSubData(m_RendererID, newRendererID, 0, 0, sz);

  std::byte *newMappedData{};

  if (m_Usage == RHI::BufferUsage::Stream) {
    constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    newMappedData = static_cast<std::byte *>(glMapNamedBufferRange(newRendererID, 0, size, flags));
  }

  Unmap();
  glDeleteBuffers(1, &m_RendererID);

  m_RendererID = newRendererID;
  m_MappedData = newMappedData;
  m_Size = size;
}

void Buffer::SetData(const RHI::BufferData &data, std::size_t offset) noexcept {
  if (data.empty())
    return;

  switch (m_Usage) {
  case RHI::BufferUsage::Static:
    CORE_ASSERT(false, "Static buffers cannot be updated with SetData");
    return;

  case RHI::BufferUsage::Dynamic:
    glNamedBufferSubData(m_RendererID, offset, data.size(), data.data());
    return;
  case RHI::BufferUsage::Stream:
    std::memcpy(m_MappedData + offset, data.data(), data.size());
    return;
  }
}

void Buffer::Bind() const noexcept { glBindBuffer(JupiterToOpenGLBufferType(m_Type), m_RendererID); }

void Buffer::Unmap() noexcept {
  if (m_RendererID && m_MappedData) {
    glUnmapNamedBuffer(m_RendererID);
    m_MappedData = nullptr;
  }
}

void Buffer::Destroy() noexcept {
  Unmap();

  if (m_RendererID) {
    glDeleteBuffers(1, &m_RendererID);
    m_RendererID = 0;
  }

  m_Size = 0;
}

} // namespace Jupiter::Rendering::OpenGL
