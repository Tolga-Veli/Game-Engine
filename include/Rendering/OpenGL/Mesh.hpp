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
        m_VertexBuffer(RHI::BufferDesc{
            .type = RHI::BufferType::Vertex,
            .usage = info.usage,
            .size = info.vertexCapacity ? info.vertexCapacity
                                        : info.vertexData.size_bytes(),
            .initData = info.vertexData.data()}),
        m_IndexBuffer(RHI::BufferDesc{.type = RHI::BufferType::Index,
                                      .usage = RHI::BufferUsage::Static,
                                      .size = info.indices.size_bytes(),
                                      .initData = info.indices.data()}),
        m_Layout(info.layout),
        m_VertexCount(info.vertexData.size_bytes() / info.layout.GetStride()),
        m_IndexCount(info.indices.size()) {

    CORE_ASSERT(!info.vertexData.empty() || info.vertexCapacity > 0,
                "Mesh vertex data cannot be empty");
    CORE_ASSERT(!info.indices.empty(), "Mesh index cannot be empty");
    CORE_ASSERT(info.layout.GetStride() > 0,
                "Mesh vertex layout stride cannot be zero");
    CORE_ASSERT(info.vertexData.empty() ||
                    info.vertexData.size_bytes() % info.layout.GetStride() == 0,
                "Mesh vertex data size is not a multiple of vertex stride");

    m_VertexArray.AddVertexBuffer(m_VertexBuffer, m_Layout);
    m_VertexArray.SetIndexBuffer(m_IndexBuffer);
  }

  void SetVertexData(std::span<const std::byte> data) override {
    CORE_ASSERT(data.size_bytes() <= m_VertexBuffer.GetSize(),
                "Dynamic mesh capacity exceeded");

    m_VertexBuffer.SetData(data.data(), data.size_bytes());
    m_VertexCount = data.size_bytes() / m_Layout.GetStride();
  }

  void SetInstanceData(std::span<const std::byte> data) {
    if (!m_InstanceBuffer) {
      m_InstanceBuffer = std::make_unique<Buffer>(
          RHI::BufferDesc{.type = RHI::BufferType::Vertex,
                          .usage = RHI::BufferUsage::Dynamic,
                          .size = data.size_bytes()});

      const RHI::BufferLayout layout{
          {RHI::ShaderDataType::Float2, "position", 1},
          {RHI::ShaderDataType::Float2, "velocity", 2},
          {RHI::ShaderDataType::Float, "radius", 3}};

      m_VertexArray.AddVertexBuffer(*m_InstanceBuffer, layout, 1);
    }

    CORE_ASSERT(data.size_bytes() <= m_InstanceBuffer->GetSize(),
                "Instance buffer capacity exceeded");

    m_InstanceBuffer->SetData(data.data(), data.size_bytes());
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
  std::unique_ptr<Buffer> m_InstanceBuffer;
  RHI::BufferLayout m_Layout;
  u32 m_VertexCount{}, m_IndexCount{};
};
} // namespace Jupiter::Rendering::OpenGL
