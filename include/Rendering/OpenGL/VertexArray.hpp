#pragma once

#include "Buffer.hpp"
#include "Rendering/Backend/BufferLayout.hpp"

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
      : m_RendererID(std::exchange(other.m_RendererID, 0)) {}

  VertexArray &operator=(VertexArray &&other) noexcept {
    if (this == &other)
      return *this;

    if (m_RendererID)
      glDeleteVertexArrays(1, &m_RendererID);

    m_RendererID = std::exchange(other.m_RendererID, 0);
    return *this;
  }

  void Bind() const noexcept { glBindVertexArray(m_RendererID); }

  void AddVertexBuffer(const Buffer &buff, const RHI::BufferLayout &layout,
                       u32 div = 0);

  void SetIndexBuffer(const Buffer &buff);

  [[nodiscard]] const std::vector<u32> &GetVertexCounts() const noexcept {
    return m_VertexCounts;
  }

  [[nodiscard]] u32 GetIndexCount() const noexcept { return m_IndexCount; }

  [[nodiscard]]
  u32 GetRendererID() const noexcept {
    return m_RendererID;
  }

private:
  u32 m_RendererID{}, m_NextBinding{}, m_IndexCount{};
  std::vector<u32> m_VertexCounts;
};
} // namespace Jupiter::Rendering::OpenGL
