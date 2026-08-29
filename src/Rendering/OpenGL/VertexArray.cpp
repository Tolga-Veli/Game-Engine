#include "Core/Utils.hpp"
#include "Core/Assert.hpp"

#include "Rendering/OpenGL/VertexArray.hpp"

namespace Jupiter::Rendering::OpenGL {

void VertexArray::AddVertexBuffer(const Ref<IVertexBuffer> &vertexBuffer,
                                  u32 divisor) {
  CORE_ASSERT(!vertexBuffer->GetLayout().GetElements().empty(),
              "Vertex Buffer is empty");

  Bind();
  vertexBuffer->Bind();

  const auto layout = vertexBuffer->GetLayout();
  for (const auto &element : layout) {
    const auto &type = element.type;

    u32 cnt = GetShaderDataTypeComponentCount(type);
    if (type == ShaderDataType::Mat3 || type == ShaderDataType::Mat4) {
      u8 columnCount = (type == ShaderDataType::Mat3) ? 3 : 4;
      for (u8 i = 0; i < columnCount; i++) {
        glEnableVertexAttribArray(element.location + i);
        glVertexAttribPointer(
            element.location + i, columnCount, JupiterToOpenGLBaseType(type),
            element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
            reinterpret_cast<const void *>(static_cast<uintptr_t>(
                element.offset + sizeof(float) * columnCount * i)));

        if (divisor > 0)
          glVertexAttribDivisor(element.location + i, divisor);
      }
    } else {
      glEnableVertexAttribArray(element.location);
      glVertexAttribPointer(
          element.location, cnt, JupiterToOpenGLBaseType(type),
          element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
          reinterpret_cast<const void *>(
              static_cast<uintptr_t>(element.offset)));

      if (divisor > 0)
        glVertexAttribDivisor(element.location, divisor);
    }
  }

  m_VertexBuffers.push_back(vertexBuffer);
}

void VertexArray::SetIndexBuffer(const Ref<IIndexBuffer> &indexBuffer) {
  Bind();
  indexBuffer->Bind();
  m_IndexBuffer = indexBuffer;
}

} // namespace Jupiter::Rendering::OpenGL
