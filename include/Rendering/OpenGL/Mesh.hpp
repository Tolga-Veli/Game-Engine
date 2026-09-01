#pragma once

#include "Rendering/Backend/BufferLayout.hpp"
#include "Rendering/Backend/IBuffer.hpp"
#include "Rendering/Backend/IMesh.hpp"

#include "Rendering/OpenGL/VertexArray.hpp"
#include "Rendering/OpenGL/Buffer.hpp"

namespace Jupiter::Rendering::OpenGL {
class Mesh final : public RHI::IMesh {
public:
  explicit Mesh(const RHI::MeshCreateInfo &info)
      : m_VertexArray(),
        m_VertexBuffer(RHI::BufferDesc{.type = RHI::BufferType::Vertex,
                                       .usage = RHI::BufferUsage::Static,
                                       .size = info.vertexData.size_bytes(),
                                       .initData = info.vertexData.data()}),
        m_IndexBuffer(RHI::BufferDesc{.type = RHI::BufferType::Index,
                                      .usage = RHI::BufferUsage::Static,
                                      .size = info.indices.size_bytes(),
                                      .initData = info.indices.data()}),
        m_Layout(info.layout),
        m_VertexCount(info.vertexData.size_bytes() / info.layout.GetStride()),
        m_IndexCount(info.indices.size()) {

    CORE_ASSERT(!info.vertexData.empty(), "Mesh vertex data cannot be empty");
    CORE_ASSERT(!info.indices.empty(), "Mesh index cannot be empty");
    CORE_ASSERT(info.layout.GetStride() > 0,
                "Mesh vertex layout stride cannot be zero");
    CORE_ASSERT(info.vertexData.size_bytes() % info.layout.GetStride() == 0,
                "Mesh vertex data size is not a multiple of vertex stride");

    m_VertexArray.AddVertexBuffer(m_VertexBuffer, m_Layout);
    m_VertexArray.SetIndexBuffer(m_IndexBuffer);
  }

  ~Mesh() noexcept override = default;

  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;
  Mesh(Mesh &&) noexcept = default;
  Mesh &operator=(Mesh &&) noexcept = default;

  [[nodiscard]]
  virtual u32 GetVertexCount() const noexcept override {
    return m_VertexCount;
  }

  [[nodiscard]]
  virtual u32 GetIndexCount() const noexcept override {
    return m_IndexCount;
  }

  [[nodiscard]]
  const VertexArray &GetVertexArray() const noexcept {
    return m_VertexArray;
  }

private:
  VertexArray m_VertexArray;
  Buffer m_VertexBuffer, m_IndexBuffer;
  RHI::BufferLayout m_Layout;
  u32 m_VertexCount{}, m_IndexCount{};
};
} // namespace Jupiter::Rendering::OpenGL
