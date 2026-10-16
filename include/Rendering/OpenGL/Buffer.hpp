#pragma once

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
  }
  return GL_NONE;
}

inline GLbitfield JupiterToOpenGLBufferUsage(RHI::BufferUsage usage) noexcept {
  using enum RHI::BufferUsage;

  switch (usage) {
  case Static:
    return GL_DYNAMIC_STORAGE_BIT;
  case Dynamic:
    // Allows glNamedBufferSubData();
    return GL_DYNAMIC_STORAGE_BIT;
  case Stream:
    return GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  }
  return GL_NONE;
}

class Buffer final : public RHI::IBuffer {
public:
  explicit Buffer(const RHI::BufferDesc &desc, const RHI::BufferData &data) noexcept;
  ~Buffer() noexcept override;

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  Buffer(Buffer &&other) noexcept;
  Buffer &operator=(Buffer &&other) noexcept;

  void Reserve(std::size_t size, bool preserve = true) noexcept;
  void SetData(const RHI::BufferData &data, std::size_t offset = 0) noexcept override;
  void Bind() const noexcept;
  void MarkUsed() noexcept override;

  [[nodiscard]] RHI::BufferType GetType() const noexcept { return m_Type; }
  [[nodiscard]] RHI::BufferUsage GetUsage() const noexcept { return m_Usage; }
  [[nodiscard]] RHI::DataType GetDataType() const noexcept { return m_DataType; }
  [[nodiscard]] std::size_t GetSize() const noexcept { return m_Size; }
  [[nodiscard]] u32 GetRendererID() const noexcept { return m_RendererID; }

private:
  u32 m_RendererID{};
  RHI::BufferType m_Type{};
  RHI::BufferUsage m_Usage{};
  RHI::DataType m_DataType{RHI::DataType::None};

  std::size_t m_Size{};
  std::byte *m_MappedData{};
  GLsync m_LastUse{};

  void Allocate(std::size_t size, const void *initialData) noexcept;

  void Destroy() noexcept;
  void Unmap() noexcept;
  void WaitForGPU() noexcept;
};
} // namespace Jupiter::Rendering::OpenGL
