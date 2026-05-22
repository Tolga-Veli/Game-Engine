#include "Core/Utils.hpp"
#include "Renderer/Backend/IVertexArray.hpp"
#include "Renderer/OpenGL/VertexArray.hpp"
#include "Renderer/Renderer.hpp"

namespace Jupiter::Renderer {

void GLVertexArray::AddVertexBuffer(const Ref<IVertexBuffer> &vertexBuffer) {
  if (vertexBuffer->GetLayout().GetElements().empty())
    return;

  Bind();
  vertexBuffer->Bind();

  const auto layout = vertexBuffer->GetLayout();
  for (const auto &element : layout) {
    const auto &type = element.type;

    u32 cnt = GetShaderDataTypeComponentCount(type);
    if (type == ShaderDataType::Mat3 || type == ShaderDataType::Mat4) {
      uint8_t columnCount = (type == ShaderDataType::Mat3) ? 3 : 4;
      for (uint8_t i = 0; i < columnCount; i++) {
        glEnableVertexAttribArray(element.location + i);
        glVertexAttribPointer(
            element.location + i, columnCount, JupiterToOpenGLBaseType(type),
            element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
            reinterpret_cast<const void *>(static_cast<uintptr_t>(
                element.offset + sizeof(float) * columnCount * i)));
      }
    } else {
      glEnableVertexAttribArray(element.location);
      glVertexAttribPointer(
          element.location, cnt, JupiterToOpenGLBaseType(type),
          element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
          reinterpret_cast<const void *>(
              static_cast<uintptr_t>(element.offset)));
    }
  }

  m_VertexBuffers.push_back(vertexBuffer);
}

void GLVertexArray::SetIndexBuffer(const Ref<IIndexBuffer> &indexBuffer) {
  Bind();
  indexBuffer->Bind();
  m_IndexBuffer = indexBuffer;
}

} // namespace Jupiter::Renderer
