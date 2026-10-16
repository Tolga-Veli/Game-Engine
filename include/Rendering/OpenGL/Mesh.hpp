#pragma once

#include "Rendering/Backend/BufferLayout.hpp"
#include "Rendering/Backend/IBuffer.hpp"
#include "Rendering/Backend/IMesh.hpp"

#include "Rendering/OpenGL/VertexArray.hpp"
#include "Rendering/OpenGL/Buffer.hpp"

namespace Jupiter::Rendering::OpenGL {
class Mesh final : public RHI::IMesh {
public:
  explicit Mesh(const RHI::MeshDesc &desc);
  ~Mesh() noexcept override = default;

  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;
  Mesh(Mesh &&) = delete;
  Mesh &operator=(Mesh &&) = delete;

  bool SetGeometry(const RHI::BufferData &vertices, const RHI::BufferData &indices) override;
  bool UpdateVertices(const RHI::BufferData &vertices) override;
  bool UpdateIndices(const RHI::BufferData &indices) override;
  bool UpdateInstances(const RHI::BufferData &instances);
  void MarkUsed() noexcept;

  [[nodiscard]] bool HasInstances() const noexcept { return m_InstanceBuffer != nullptr; }
  [[nodiscard]] u32 GetInstanceCount() const noexcept { return m_InstanceCount; }
  [[nodiscard]] bool HasValidIndices() const noexcept { return !m_IndexCount || m_MaxIndex < m_VertexCount; }

  [[nodiscard]] const RHI::BufferLayout &GetLayout() const noexcept override { return m_Layout; }

  [[nodiscard]] u32 GetVertexCount() const noexcept override { return m_VertexCount; }
  [[nodiscard]] u32 GetVertexCapacity() const noexcept override { return m_VertexCapacity; }
  [[nodiscard]] u32 GetIndexCount() const noexcept override { return m_IndexCount; }
  [[nodiscard]] u32 GetIndexCapacity() const noexcept override { return m_IndexCapacity; }
  [[nodiscard]] const VertexArray &GetVertexArray() const noexcept { return m_VertexArray; }

private:
  RHI::BufferLayout m_Layout;
  VertexArray m_VertexArray;
  Buffer m_VertexBuffer, m_IndexBuffer;
  Scope<Buffer> m_InstanceBuffer;
  u32 m_VertexCount{}, m_VertexCapacity{}, m_IndexCount{}, m_IndexCapacity{};
  u32 m_InstanceCapacity{};
  u32 m_InstanceCount{}, m_MaxIndex{};
  RHI::BufferLayout m_InstanceLayout;

  static u32 GrowCapacity(u32 curr, u32 required) noexcept;

  [[nodiscard]] bool IsValidVertexData(const RHI::BufferData &data) const noexcept;
  [[nodiscard]] bool IsValidIndexData(const RHI::BufferData &data) const noexcept;
  [[nodiscard]] static u32 MaxIndex(const RHI::BufferData &data) noexcept;

  void EnsureVertexCapacity(u32 sz);
  void EnsureIndexCapacity(u32 sz);
};
} // namespace Jupiter::Rendering::OpenGL
