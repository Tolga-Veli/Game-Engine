#include "Rendering/OpenGL/Mesh.hpp"

#include "Core/Utils.hpp"

#include <algorithm>
#include <cstring>
#include <limits>

namespace Jupiter::Rendering::OpenGL {

namespace {
RHI::BufferDesc WithCapacity(RHI::BufferDesc desc, u32 capacity, u32 stride) {
  desc.bytes = std::max(desc.bytes, static_cast<std::size_t>(capacity) * stride);
  return desc;
}
} // namespace

Mesh::Mesh(const RHI::MeshDesc &desc)
    : m_Layout(desc.layout), m_VertexArray(),
      m_VertexBuffer(WithCapacity(desc.vertices, desc.vertexCap, desc.layout.GetStride()), desc.vertexData.as_span()),
      m_IndexBuffer(WithCapacity(desc.indices, desc.indexCap, sizeof(u32)), desc.indexData.as_span()),
      m_VertexCapacity(desc.vertexCap), m_IndexCapacity(desc.indexCap), m_InstanceCapacity(desc.instanceCap),
      m_InstanceLayout(desc.instanceLayout) {
  m_VertexArray.AddVertexBuffer(m_VertexBuffer, m_Layout);
  m_VertexArray.SetIndexBuffer(m_IndexBuffer);
  m_VertexCount = m_Layout.GetStride() ? (desc.vertexData.size() / m_Layout.GetStride()) : 0;
  m_IndexCount = desc.indexData.size() / sizeof(u32);
  m_VertexCapacity = std::max(m_VertexCapacity, m_VertexCount);
  m_IndexCapacity = std::max(m_IndexCapacity, m_IndexCount);
  m_MaxIndex = MaxIndex(desc.indexData.as_span());

  if (m_InstanceLayout.GetStride()) {
    RHI::BufferDesc instanceDesc{
        RHI::BufferType::Vertex, RHI::BufferUsage::Dynamic, RHI::DataType::None,
        std::max<std::size_t>(1, static_cast<std::size_t>(m_InstanceCapacity) * m_InstanceLayout.GetStride())};
    m_InstanceBuffer = CreateScope<Buffer>(instanceDesc, RHI::BufferData{});
    m_VertexArray.AddVertexBuffer(*m_InstanceBuffer, m_InstanceLayout, 1);
  }
}

u32 Mesh::GrowCapacity(u32 curr, u32 sz) noexcept {
  if (sz <= curr)
    return curr;

  u32 cap = std::max(curr, 1u);
  while (cap < sz)
    if (cap > std::numeric_limits<u32>::max() / 2)
      return sz;
    else
      cap *= 2;

  return cap;
}

bool Mesh::IsValidVertexData(const RHI::BufferData &data) const noexcept {
  return m_Layout.GetStride() && data.size() % m_Layout.GetStride() == 0 &&
         data.size() / m_Layout.GetStride() <= std::numeric_limits<u32>::max();
}

bool Mesh::IsValidIndexData(const RHI::BufferData &data) const noexcept {
  return data.size() % sizeof(u32) == 0 && data.size() / sizeof(u32) <= std::numeric_limits<u32>::max();
}

void Mesh::EnsureVertexCapacity(u32 required) {
  if (required <= m_VertexCapacity)
    return;

  const u32 newCapacity = GrowCapacity(m_VertexCapacity, required);
  const std::size_t newBytes = static_cast<std::size_t>(newCapacity) * m_Layout.GetStride();

  m_VertexBuffer.Reserve(newBytes, false);
  m_VertexArray.ReplaceVertexBuffer(0, m_VertexBuffer, m_Layout);
  m_VertexCapacity = newCapacity;
}

void Mesh::EnsureIndexCapacity(u32 required) {
  if (required <= m_IndexCapacity)
    return;

  const u32 newCapacity = GrowCapacity(m_IndexCapacity, required);
  const std::size_t newBytes = static_cast<std::size_t>(newCapacity) * sizeof(u32);

  m_IndexBuffer.Reserve(newBytes, false);
  m_VertexArray.SetIndexBuffer(m_IndexBuffer);
  m_IndexCapacity = newCapacity;
}

bool Mesh::SetGeometry(const RHI::BufferData &vertices, const RHI::BufferData &indices) {
  if (m_VertexBuffer.GetUsage() == RHI::BufferUsage::Static || m_IndexBuffer.GetUsage() == RHI::BufferUsage::Static ||
      !IsValidVertexData(vertices) || !IsValidIndexData(indices))
    return false;
  const u32 vertexCount = static_cast<u32>(vertices.size() / m_Layout.GetStride());
  const u32 indexCount = static_cast<u32>(indices.size() / sizeof(u32));
  const u32 maxIndex = MaxIndex(indices);
  if (indexCount && maxIndex >= vertexCount)
    return false;

  EnsureVertexCapacity(vertexCount);
  EnsureIndexCapacity(indexCount);

  m_VertexBuffer.SetData(vertices);
  m_IndexBuffer.SetData(indices);

  m_VertexCount = vertexCount;
  m_IndexCount = indexCount;
  m_MaxIndex = maxIndex;

  return true;
}

bool Mesh::UpdateVertices(const RHI::BufferData &vertices) {
  if (m_VertexBuffer.GetUsage() == RHI::BufferUsage::Static || !IsValidVertexData(vertices))
    return false;

  const u32 vertexCount = static_cast<u32>(vertices.size() / m_Layout.GetStride());
  if (m_IndexCount && m_MaxIndex >= vertexCount)
    return false;

  EnsureVertexCapacity(vertexCount);

  m_VertexBuffer.SetData(vertices);
  m_VertexCount = vertexCount;
  return true;
}

bool Mesh::UpdateIndices(const RHI::BufferData &indices) {
  if (m_IndexBuffer.GetUsage() == RHI::BufferUsage::Static || !IsValidIndexData(indices))
    return false;

  const u32 indexCount = static_cast<u32>(indices.size() / sizeof(u32));
  const u32 maxIndex = MaxIndex(indices);
  if (indexCount && maxIndex >= m_VertexCount)
    return false;

  EnsureIndexCapacity(indexCount);

  m_IndexBuffer.SetData(indices);
  m_IndexCount = indexCount;
  m_MaxIndex = maxIndex;
  return true;
}

bool Mesh::UpdateInstances(const RHI::BufferData &instances) {
  if (!m_InstanceBuffer || !m_InstanceLayout.GetStride() || instances.size() % m_InstanceLayout.GetStride())
    return false;

  const auto count = instances.size() / m_InstanceLayout.GetStride();
  if (count > std::numeric_limits<u32>::max())
    return false;
  if (count > m_InstanceCapacity) {
    m_InstanceCapacity = GrowCapacity(m_InstanceCapacity, static_cast<u32>(count));
    m_InstanceBuffer->Reserve(static_cast<std::size_t>(m_InstanceCapacity) * m_InstanceLayout.GetStride(), false);
    m_VertexArray.ReplaceVertexBuffer(1, *m_InstanceBuffer, m_InstanceLayout);
  }
  m_InstanceBuffer->SetData(instances);
  m_InstanceCount = static_cast<u32>(count);
  return true;
}

u32 Mesh::MaxIndex(const RHI::BufferData &data) noexcept {
  u32 maximum{};
  for (std::size_t offset = 0; offset + sizeof(u32) <= data.size(); offset += sizeof(u32)) {
    u32 index;
    std::memcpy(&index, data.data() + offset, sizeof(index));
    maximum = std::max(maximum, index);
  }
  return maximum;
}

void Mesh::MarkUsed() noexcept {
  m_VertexBuffer.MarkUsed();
  m_IndexBuffer.MarkUsed();
  if (m_InstanceBuffer)
    m_InstanceBuffer->MarkUsed();
}
} // namespace Jupiter::Rendering::OpenGL
