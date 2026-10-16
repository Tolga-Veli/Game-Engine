#include "Core/Assert.hpp"
#include "Core/Utils.hpp"

#include "Rendering/Backend/BufferLayout.hpp"
#include "Rendering/Backend/IBuffer.hpp"

#include "Rendering/OpenGL/Buffer.hpp"
#include "Rendering/OpenGL/OpenGL-Utils.hpp"
#include "Rendering/OpenGL/VertexArray.hpp"

namespace Jupiter::Rendering::OpenGL {

void VertexArray::Bind() const noexcept {
  CORE_ASSERT(m_RendererID != 0, "Cannot bind an invalid vertex array");
  glBindVertexArray(m_RendererID);
}

void VertexArray::AddVertexBuffer(const Buffer &buffer, const RHI::BufferLayout &layout, u32 divisor) {
  CORE_ASSERT(m_RendererID != 0, "Cannot modify an invalid vertex array");
  CORE_ASSERT(buffer.GetType() == RHI::BufferType::Vertex, "Expected a vertex buffer");
  CORE_ASSERT(layout.GetStride() > 0, "Vertex layout stride must be non-zero");

  const u32 binding = m_NextBinding++;
  glVertexArrayVertexBuffer(m_RendererID, binding, buffer.GetRendererID(), 0, layout.GetStride());

  if (divisor > 0)
    glVertexArrayBindingDivisor(m_RendererID, binding, divisor);

  for (const auto &element : layout) {
    const auto type = element.type;

    CORE_ASSERT(type != RHI::DataType::None && type != RHI::DataType::Texture2D && type != RHI::DataType::Count,
                "Unsupported vertex attribute type");

    const GLenum glType = JupiterToOpenGLShaderDataType(type);
    if (type == RHI::DataType::Mat3 || type == RHI::DataType::Mat4) {
      const u32 columnCount = type == RHI::DataType::Mat3 ? 3 : 4, columnSize = RHI::DataTypeSize(type) / columnCount;

      for (u32 column = 0; column < columnCount; ++column) {
        const u32 location = element.location + column, offset = element.offset + column * columnSize;

        glEnableVertexArrayAttrib(m_RendererID, location);
        glVertexArrayAttribFormat(m_RendererID, location, static_cast<GLint>(columnCount), glType, element.normalized,
                                  offset);
        glVertexArrayAttribBinding(m_RendererID, location, binding);
      }

      continue;
    }

    const u32 componentCount = RHI::DataTypeComponentCount(type);

    glEnableVertexArrayAttrib(m_RendererID, element.location);

    switch (type) {
    case RHI::DataType::Int:
    case RHI::DataType::Int2:
    case RHI::DataType::Int3:
    case RHI::DataType::Int4:
    case RHI::DataType::UInt:
    case RHI::DataType::UInt2:
    case RHI::DataType::UInt3:
    case RHI::DataType::UInt4:
      glVertexArrayAttribIFormat(m_RendererID, element.location, static_cast<GLint>(componentCount), glType,
                                 element.offset);
      break;

    default:
      glVertexArrayAttribFormat(m_RendererID, element.location, static_cast<GLint>(componentCount), glType,
                                element.normalized, element.offset);
      break;
    }

    glVertexArrayAttribBinding(m_RendererID, element.location, binding);
  }
}

void VertexArray::ReplaceVertexBuffer(u32 binding, const Buffer &vertexBuffer, const RHI::BufferLayout &layout) {
  CORE_ASSERT(m_RendererID != 0, "Cannot modify an invalid vertex array");
  CORE_ASSERT(binding < m_NextBinding, "Vertex buffer binding does not exist");
  CORE_ASSERT(vertexBuffer.GetType() == RHI::BufferType::Vertex, "Wrong buffer type passed into vertex array");
  CORE_ASSERT(layout.GetStride() > 0, "Vertex layout stride must be non-zero");

  glVertexArrayVertexBuffer(m_RendererID, binding, vertexBuffer.GetRendererID(), 0, layout.GetStride());
}

void VertexArray::SetIndexBuffer(const Buffer &buffer) {
  CORE_ASSERT(m_RendererID != 0, "Cannot modify an invalid vertex array");
  CORE_ASSERT(buffer.GetType() == RHI::BufferType::Index, "Expected an index buffer");
  CORE_ASSERT(buffer.GetSize() % sizeof(u32) == 0, "Index buffer capacity must be a multiple of sizeof(u32)");

  glVertexArrayElementBuffer(m_RendererID, buffer.GetRendererID());
}

} // namespace Jupiter::Rendering::OpenGL
