#pragma once

#include "Core/Assert.hpp"
#include "Rendering/Backend/IBuffer.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

inline GLenum JupiterToOpenGLBufferType(RHI::BufferType type) {
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
  default:
    UNREACHABLE("Uknown RHI::BufferType");
    return GL_ARRAY_BUFFER;
  }
}

inline GLbitfield JupiterToOpenGLBufferUsage(RHI::BufferUsage usage) noexcept {
  using enum RHI::BufferUsage;

  switch (usage) {
  case Static:
    // Immutable after creation
    return GL_NONE;
  case Dynamic:
    // Allows glNamedBufferSubData();
    return GL_DYNAMIC_STORAGE_BIT;
  case Stream:
    return GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  default:
    UNREACHABLE("Unknown RHI::BufferUsage");
    return GL_NONE;
  }
}

class Buffer final : public RHI::IBuffer {
public:
  explicit Buffer(const RHI::BufferDesc &desc) noexcept;
  ~Buffer() noexcept override;

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;
  Buffer(Buffer &&other) noexcept;
  Buffer &operator=(Buffer &&other) noexcept;

  [[nodiscard]]
  RHI::BufferType GetType() const noexcept {
    return m_Desc.type;
  }

  [[nodiscard]]
  RHI::BufferUsage GetUsage() const noexcept {
    return m_Desc.usage;
  }

  [[nodiscard]] u32 GetSize() const noexcept { return m_Desc.size; }

  [[nodiscard]]
  u32 GetRendererID() const noexcept {
    return m_RendererID;
  }

  [[nodiscard]]
  bool IsMapped() const noexcept {
    return m_MappedData != nullptr;
  }

  [[nodiscard]]
  void *GetMappedData() noexcept {
    return m_MappedData;
  }

  [[nodiscard]]
  const void *GetMappedData() const noexcept {
    return m_MappedData;
  }

  void Bind() const noexcept;
  void Unbind() const noexcept;
  void SetData(const void *data, u32 size, u32 offset = 0) noexcept override;

private:
  u32 m_RendererID{};
  RHI::BufferDesc m_Desc;
  void *m_MappedData{};

  void Destroy() noexcept;
};
} // namespace Jupiter::Rendering::OpenGL
