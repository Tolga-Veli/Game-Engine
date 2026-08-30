#pragma once

#include "Rendering/Backend/BufferLayout.hpp"
#include "Rendering/Backend/IBuffer.hpp"
#include "Rendering/Backend/IMesh.hpp"

#include "Rendering/Backend/RenderUtils.hpp"
#include "Rendering/OpenGL/VertexArray.hpp"
#include "Rendering/OpenGL/Buffer.hpp"

namespace Jupiter::Rendering::OpenGL {
class Mesh final : public IMesh {
public:
  Mesh(const std::vector<float> &vertices, const std::vector<u32> &indices,
       const RHI::BufferLayout &layout)
      : m_VertexArray(),
        m_VertexBuffer(RHI::BufferDesc{.type = RHI::BufferType::Vertex,
                                       .usage = RHI::BufferUsage::Static,
                                       .size = vertices.size()}),
        m_IndexBuffer(RHI::BufferDesc{.type = RHI::BufferType::Index,
                                      .usage = RHI::BufferUsage::Static,
                                      .size = indices.size()}) {
    m_VertexBuffer.SetLayout(layout);
    m_VertexArray.AddVertexBuffer(m_VertexBuffer);
  }

private:
  VertexArray m_VertexArray;
  Buffer m_VertexBuffer;
  Buffer m_IndexBuffer;
};
} // namespace Jupiter::Rendering::OpenGL
