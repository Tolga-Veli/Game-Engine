#pragma once

#include "Buffer.hpp"

#include "Rendering/Backend/BufferLayout.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
class VertexArray {
public:
  VertexArray() noexcept;
  ~VertexArray() noexcept;

  void Bind() noexcept;
  void Unbind() noexcept;

  void ConfigureVertexBuffer(const Buffer &buff,
                             const RHI::BufferLayout &layout);

  void ConfigureIndexBuffer(const Buffer &buff);

private:
  u32 m_RendererID{};
};
} // namespace Jupiter::Rendering::OpenGL
