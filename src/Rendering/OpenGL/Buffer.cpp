#include "Rendering/OpenGL/Buffer.hpp"
#include "Rendering/Backend/IBuffer.hpp"

#include <cstring>
#include <algorithm>
#include <limits>

namespace Jupiter::Rendering::OpenGL {
Buffer::Buffer(const RHI::BufferDesc &desc, const RHI::BufferData &data) noexcept
    : m_Type(desc.type), m_Usage(desc.usage), m_DataType(desc.dtype) {
  const auto minimum = desc.type == RHI::BufferType::Index ? sizeof(u32) : std::size_t{1};
  Allocate(std::max(minimum, std::max(desc.bytes, data.size())), nullptr);
  if (!data.empty())
    glNamedBufferSubData(m_RendererID, 0, data.size(), data.data());
}

Buffer::~Buffer() noexcept { Destroy(); }

Buffer::Buffer(Buffer &&other) noexcept
    : m_RendererID(std::exchange(other.m_RendererID, 0)), m_Type(other.m_Type), m_Usage(other.m_Usage),
      m_DataType(other.m_DataType), m_Size(std::exchange(other.m_Size, 0)),
      m_MappedData(std::exchange(other.m_MappedData, nullptr)), m_LastUse(std::exchange(other.m_LastUse, nullptr)) {}

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
  m_LastUse = std::exchange(other.m_LastUse, nullptr);

  return *this;
}

void Buffer::Allocate(std::size_t size, const void *initialData) noexcept {
  CORE_VERIFY(size <= static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()),
              "Buffer size exceeds GL limits");
  glCreateBuffers(1, &m_RendererID);
  glNamedBufferStorage(m_RendererID, size, initialData, JupiterToOpenGLBufferUsage(m_Usage));
  m_Size = size;

  if (m_Usage == RHI::BufferUsage::Stream) {
    constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    m_MappedData = static_cast<std::byte *>(glMapNamedBufferRange(m_RendererID, 0, m_Size, flags));
    CORE_VERIFY(m_MappedData, "Failed to map stream buffer");
  }
}

void Buffer::Reserve(std::size_t size, bool preserve) noexcept {
  if (size <= m_Size)
    return;
  CORE_VERIFY(size <= static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()),
              "Buffer size exceeds GL limits");
  if (preserve)
    WaitForGPU();
  else if (m_LastUse) {
    glDeleteSync(m_LastUse);
    m_LastUse = nullptr;
  }

  u32 newRendererID{};
  glCreateBuffers(1, &newRendererID);

  glNamedBufferStorage(newRendererID, size, nullptr, JupiterToOpenGLBufferUsage(m_Usage));

  const std::size_t sz = std::min(m_Size, size);
  if (preserve && sz > 0)
    glCopyNamedBufferSubData(m_RendererID, newRendererID, 0, 0, sz);

  std::byte *newMappedData{};

  if (m_Usage == RHI::BufferUsage::Stream) {
    // A preserved copy writes the new storage asynchronously too.
    if (preserve && sz > 0) {
      m_LastUse = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
      CORE_VERIFY(m_LastUse, "Failed to fence buffer copy");
      WaitForGPU();
    }
    constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    newMappedData = static_cast<std::byte *>(glMapNamedBufferRange(newRendererID, 0, size, flags));
    CORE_VERIFY(newMappedData, "Failed to map grown stream buffer");
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
  CORE_VERIFY(offset <= m_Size && data.size() <= m_Size - offset, "Buffer upload exceeds capacity");

  switch (m_Usage) {
  case RHI::BufferUsage::Static:
    CORE_ASSERT(false, "Static buffers cannot be updated with SetData");
    return;

  case RHI::BufferUsage::Dynamic:
    glNamedBufferSubData(m_RendererID, offset, data.size(), data.data());
    return;
  case RHI::BufferUsage::Stream:
    WaitForGPU();
    std::memcpy(m_MappedData + offset, data.data(), data.size());
    return;
  }
}

void Buffer::Bind() const noexcept { glBindBuffer(JupiterToOpenGLBufferType(m_Type), m_RendererID); }

void Buffer::MarkUsed() noexcept {
  if (m_Usage != RHI::BufferUsage::Stream)
    return;
  if (m_LastUse)
    glDeleteSync(m_LastUse);
  m_LastUse = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  CORE_VERIFY(m_LastUse, "Failed to fence stream buffer");
}

void Buffer::WaitForGPU() noexcept {
  if (!m_LastUse)
    return;
  GLenum result;
  do {
    result = glClientWaitSync(m_LastUse, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
    CORE_VERIFY(result != GL_WAIT_FAILED, "Failed to wait for stream buffer");
  } while (result == GL_TIMEOUT_EXPIRED);
  glDeleteSync(m_LastUse);
  m_LastUse = nullptr;
}

void Buffer::Unmap() noexcept {
  if (m_RendererID && m_MappedData) {
    glUnmapNamedBuffer(m_RendererID);
    m_MappedData = nullptr;
  }
}

void Buffer::Destroy() noexcept {
  if (m_LastUse) {
    glDeleteSync(m_LastUse);
    m_LastUse = nullptr;
  }
  Unmap();

  if (m_RendererID) {
    glDeleteBuffers(1, &m_RendererID);
    m_RendererID = 0;
  }

  m_Size = 0;
}

} // namespace Jupiter::Rendering::OpenGL
