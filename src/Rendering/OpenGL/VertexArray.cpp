#include "Core/Assert.hpp"
#include "Core/Utils.hpp"

#include "Rendering/Backend/BufferLayout.hpp"
#include "Rendering/Backend/IBuffer.hpp"

#include "Rendering/OpenGL/Buffer.hpp"
#include "Rendering/OpenGL/OpenGL-Utils.hpp"
#include "Rendering/OpenGL/VertexArray.hpp"

namespace Jupiter::Rendering::OpenGL {

void VertexArray::AddVertexBuffer(const Buffer &vertexBuffer,
                                  const RHI::BufferLayout &layout,
                                  u32 divisor) {
  CORE_ASSERT(vertexBuffer.GetType() == RHI::BufferType::Vertex,
              "Wrong buffer type passed into vertex array");

  const u32 binding = m_NextBinding++;
  glVertexArrayVertexBuffer(m_RendererID, binding, vertexBuffer.GetRendererID(),
                            0, layout.GetStride());

  if (divisor > 0)
    glVertexArrayBindingDivisor(m_RendererID, binding, divisor);

  for (const auto &element : layout) {
    const auto type = element.type;
    if (type == RHI::ShaderDataType::Mat3 ||
        type == RHI::ShaderDataType::Mat4) {

      const u32 columnCount = type == RHI::ShaderDataType::Mat3 ? 3 : 4;
      for (u32 column = 0; column < columnCount; ++column) {
        const u32 location = element.location + column,
                  offset =
                      element.offset + sizeof(float) * columnCount * column;

        glEnableVertexArrayAttrib(m_RendererID, location);

        glVertexArrayAttribFormat(m_RendererID, location, columnCount, GL_FLOAT,
                                  element.normalized, offset);

        glVertexArrayAttribBinding(m_RendererID, location, binding);
      }

      continue;
    }

    const u32 location = element.location,
              count = ShaderDataTypeComponentCount(type);

    glEnableVertexArrayAttrib(m_RendererID, location);
    if (type == RHI::ShaderDataType::Int || type == RHI::ShaderDataType::UInt)
      glVertexArrayAttribIFormat(m_RendererID, location, count,
                                 JupiterToOpenGLBaseType(type), element.offset);

    else
      glVertexArrayAttribFormat(m_RendererID, location, count,
                                JupiterToOpenGLBaseType(type),
                                element.normalized, element.offset);

    glVertexArrayAttribBinding(m_RendererID, location, binding);
  }

  m_VertexCounts.push_back(vertexBuffer.GetSize() / layout.GetStride());
}

void VertexArray::ReplaceVertexBuffer(u32 binding, const Buffer &vertexBuffer,
                                      const RHI::BufferLayout &layout) {
  CORE_ASSERT(binding < m_NextBinding, "Vertex buffer binding does not exist");
  CORE_ASSERT(vertexBuffer.GetType() == RHI::BufferType::Vertex,
              "Wrong buffer type passed into vertex array");

  glVertexArrayVertexBuffer(m_RendererID, binding,
                            vertexBuffer.GetRendererID(), 0,
                            layout.GetStride());
  m_VertexCounts[binding] = vertexBuffer.GetSize() / layout.GetStride();
}

void VertexArray::SetIndexBuffer(const Buffer &indexBuffer) {
  CORE_ASSERT(indexBuffer.GetType() == RHI::BufferType::Index,
              "Wrong buffer type passed into vertex array");

  glVertexArrayElementBuffer(m_RendererID, indexBuffer.GetRendererID());

  m_IndexCount = indexBuffer.GetSize() / sizeof(u32);
}

} // namespace Jupiter::Rendering::OpenGL
