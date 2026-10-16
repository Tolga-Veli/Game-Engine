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

  bool SetGeometry(const RHI::BufferDesc &desc, const RHI::BufferData &indices) override;
  bool UpdateVertices(const RHI::BufferData &vertices) override;
  bool UpdateIndices(const RHI::BufferData &indices) override;

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
  u32 m_VertexCount{}, m_VertexCapacity{}, m_IndexCount{}, m_IndexCapacity{};

  static u32 GrowCapacity(u32 curr, u32 required) noexcept;

  [[nodiscard]] bool IsValidVertexData(const RHI::BufferData &data) const noexcept;
  [[nodiscard]] bool IsValidIndexData(const RHI::BufferData &data) const noexcept;

  void EnsureVertexCapacity(u32 sz);
  void EnsureIndexCapacity(u32 sz);
};
} // namespace Jupiter::Rendering::OpenGL
