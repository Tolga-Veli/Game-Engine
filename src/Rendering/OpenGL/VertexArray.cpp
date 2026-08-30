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

  u32 binding = 0;
  glVertexArrayVertexBuffer(m_RendererID, binding, vertexBuffer.GetRendererID(),
                            0, layout.GetStride());

  if (divisor > 0)
    glVertexArrayBindingDivisor(m_RendererID, binding, divisor);

  for (const auto &element : layout) {
    const auto type = element.type;

    if (type == RHI::ShaderDataType::Mat3 ||
        type == RHI::ShaderDataType::Mat4) {

      const u32 cols = type == RHI::ShaderDataType::Mat3 ? 3 : 4;
      for (u32 col = 0; col < cols; ++col) {
        const u32 location = element.location + col;

        glEnableVertexArrayAttrib(m_RendererID, location);

        glVertexArrayAttribFormat(
            m_RendererID, location, cols, JupiterToOpenGLBaseType(type),
            element.normalized, element.offset + sizeof(float) * cols * col);

        glVertexArrayAttribBinding(m_RendererID, location, binding);
      }

      continue;
    }

    const u32 location = element.location,
              count = ShaderDataTypeComponentCount(type);

    glEnableVertexArrayAttrib(m_RendererID, location);

    if (type == RHI::ShaderDataType::Int || type == RHI::ShaderDataType::UInt) {
      glVertexArrayAttribIFormat(m_RendererID, location, count,
                                 JupiterToOpenGLBaseType(type), element.offset);

    } else
      glVertexArrayAttribFormat(m_RendererID, location, count,
                                JupiterToOpenGLBaseType(type),
                                element.normalized, element.offset);

    glVertexArrayAttribBinding(m_RendererID, location, binding);
  }
}

void VertexArray::SetIndexBuffer(const Buffer &indexBuffer) {
  CORE_ASSERT(indexBuffer.GetType() == RHI::BufferType::Index,
              "Wrong buffer type passed into vertex array");

  glVertexArrayElementBuffer(m_RendererID, indexBuffer.GetRendererID());
}

} // namespace Jupiter::Rendering::OpenGL
