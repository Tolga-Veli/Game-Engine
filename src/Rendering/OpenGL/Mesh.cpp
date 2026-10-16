#include "Rendering/OpenGL/Mesh.hpp"

#include "Core/Utils.hpp"

#include <algorithm>

namespace Jupiter::Rendering::OpenGL {

Mesh::Mesh(const RHI::MeshDesc &desc)
    : m_Layout(desc.layout), m_VertexArray(), m_VertexBuffer(desc.vertices), m_IndexBuffer(desc.indices),
      m_VertexCapacity(desc.vertexCap), m_IndexCapacity(desc.indexCap) {
  m_VertexArray.AddVertexBuffer(m_VertexBuffer, m_Layout);
  m_VertexArray.SetIndexBuffer(m_IndexBuffer);
}

u32 Mesh::GrowCapacity(u32 curr, u32 sz) noexcept {
  if (sz <= curr)
    return curr;

  u32 cap = std::max(curr, 1u);
  while (cap < sz)
    cap *= 2;

  return cap;
}

void Mesh::EnsureVertexCapacity(u32 required) {
  if (required <= m_VertexCapacity)
    return;

  const u32 newCapacity = GrowCapacity(m_VertexCapacity, required);
  const std::size_t newBytes = static_cast<std::size_t>(newCapacity) * m_Layout.GetStride();

  m_VertexBuffer.Reserve(newBytes);
  m_VertexArray.ReplaceVertexBuffer(0, m_VertexBuffer, m_Layout);
  m_VertexCapacity = newCapacity;
}

void Mesh::EnsureIndexCapacity(u32 required) {
  if (required <= m_IndexCapacity)
    return;

  const u32 newCapacity = GrowCapacity(m_IndexCapacity, required);
  const std::size_t newBytes = static_cast<std::size_t>(newCapacity) * sizeof(u32);

  m_IndexBuffer.Reserve(newBytes);
  m_VertexArray.SetIndexBuffer(m_IndexBuffer);
  m_IndexCapacity = newCapacity;
}

bool Mesh::SetGeometry(const RHI::BufferData &vertices, const RHI::BufferData &indices) {
  const u32 vertexCount = static_cast<u32>(vertices.size() / m_Layout.GetStride());
  const u32 indexCount = static_cast<u32>(indices.size() / sizeof(u32));

  EnsureVertexCapacity(vertexCount);
  EnsureIndexCapacity(indexCount);

  m_VertexBuffer.SetData(vertices);
  m_IndexBuffer.SetData(indices);

  m_VertexCount = vertexCount;
  m_IndexCount = indexCount;

  return true;
}

bool Mesh::UpdateVertices(const RHI::BufferData &vertices) {
  if (!IsValidVertexData(vertices))
    return false;

  const u32 vertexCount = static_cast<u32>(vertices.size() / m_Layout.GetStride());

  EnsureVertexCapacity(vertexCount);

  m_VertexBuffer.SetData(vertices);
  m_VertexCount = vertexCount;
  return true;
}

bool Mesh::UpdateIndices(const RHI::BufferData &indices) {
  if (!IsValidIndexData(indices))
    return false;

  const u32 indexCount = static_cast<u32>(indices.size() / sizeof(u32));

  EnsureIndexCapacity(indexCount);

  m_IndexBuffer.SetData(indices);
  m_IndexCount = indexCount;
  return true;
}
} // namespace Jupiter::Rendering::OpenGL
