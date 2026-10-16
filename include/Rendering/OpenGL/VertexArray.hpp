#pragma once

#include "Rendering/Backend/BufferLayout.hpp"
#include "Buffer.hpp"

#include <glad/gl.h>
#include <utility>

namespace Jupiter::Rendering::OpenGL {
class VertexArray {
public:
  VertexArray() noexcept { glCreateVertexArrays(1, &m_RendererID); }
  ~VertexArray() noexcept {
    if (m_RendererID)
      glDeleteVertexArrays(1, &m_RendererID);
  }

  VertexArray(const VertexArray &) = delete;
  VertexArray &operator=(const VertexArray &) = delete;

  VertexArray(VertexArray &&other) noexcept
      : m_RendererID(std::exchange(other.m_RendererID, 0)), m_NextBinding(std::exchange(other.m_NextBinding, 0)) {}

  VertexArray &operator=(VertexArray &&other) noexcept {
    if (this == &other)
      return *this;

    if (m_RendererID)
      glDeleteVertexArrays(1, &m_RendererID);

    m_RendererID = std::exchange(other.m_RendererID, 0);
    m_NextBinding = std::exchange(other.m_NextBinding, 0);
    return *this;
  }

  void Bind() const noexcept;

  void AddVertexBuffer(const Buffer &buff, const RHI::BufferLayout &layout, u32 divisor = 0);
  void ReplaceVertexBuffer(u32 binding, const Buffer &buff, const RHI::BufferLayout &layout);
  void SetIndexBuffer(const Buffer &buff);

  [[nodiscard]] u32 GetRendererID() const noexcept { return m_RendererID; }

private:
  u32 m_RendererID{}, m_NextBinding{};
};
} // namespace Jupiter::Rendering::OpenGL
