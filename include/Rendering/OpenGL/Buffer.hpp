#pragma once

#include "Core/Assert.hpp"
#include "Rendering/Backend/IBuffer.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

inline GLenum JupiterToOpenGLBuffer(RHI::BufferType type) {
  using enum RHI::BufferType;
  switch (type) {
  case Vertex:
    return GL_ARRAY_BUFFER;
  case Index:
    return GL_ELEMENT_ARRAY_BUFFER;
  case Uniform:
    return GL_UNIFORM_BUFFER;
  case Storage:
    return GL_SHADER_STORAGE_BUFFER;
  }

  UNREACHABLE("Uknown RHI::BufferType");
  return GL_ARRAY_BUFFER;
}

inline GLbitfield
JupiterToOpenGLBufferStorageFlags(RHI::BufferUsage usage) noexcept {
  using enum RHI::BufferUsage;
  switch (usage) {
  case Static:
    return GL_NONE;
  case Dynamic:
    return GL_DYNAMIC_STORAGE_BIT;
  case Stream:
    return GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  }

  CORE_ASSERT(false, "Unknown RHI::BufferUsage");
  return 0;
}

class Buffer final : public RHI::IBuffer {
public:
  explicit Buffer(const RHI::BufferDesc &desc) noexcept;
  ~Buffer() noexcept override {
    if (m_RendererID)
      glDeleteBuffers(1, &m_RendererID);
  }

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  Buffer(Buffer &&other) noexcept
      : m_RendererID(std::exchange(other.m_RendererID, 0)),
        m_Desc(other.m_Desc) {}

  Buffer &operator=(Buffer &&other) noexcept {
    if (this == &other)
      return *this;

    if (m_RendererID)
      glDeleteBuffers(1, &m_RendererID);

    m_RendererID = std::exchange(other.m_RendererID, 0);
    m_Desc = other.m_Desc;
    return *this;
  }

  [[nodiscard]]
  RHI::BufferType GetType() const noexcept {
    return m_Desc.type;
  }

  void Bind() const noexcept {
    glBindBuffer(JupiterToOpenGLBuffer(m_Desc.type), m_RendererID);
  }

  void Unbind() const noexcept;

  void SetData(const void *data, u32 size, u32 offset = 0) noexcept override;

  [[nodiscard]]
  u32 GetRendererID() const noexcept {
    return m_RendererID;
  }

private:
  u32 m_RendererID{};
  RHI::BufferDesc m_Desc;
};
} // namespace Jupiter::Rendering::OpenGL
