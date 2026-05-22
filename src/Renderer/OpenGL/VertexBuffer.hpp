#pragma once

#include "OpenGL-Utils.hpp"
#include "Renderer/Backend/IVertexBuffer.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Jupiter::Renderer {

class GLVertexBuffer : public IVertexBuffer {
public:
  GLVertexBuffer() = default;
  GLVertexBuffer(u32 size, BufferUsage usage) : m_Size(size) {
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr,
                 JupiterToOpenGLBufferUsage(usage));
  }

  GLVertexBuffer(const void *data, u32 size, BufferUsage usage) : m_Size(size) {
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, data,
                 JupiterToOpenGLBufferUsage(usage));
  }

  u32 GetVertexCount() const override {
    if (m_Layout.GetStride() == 0)
      return 0;
    return m_Size / m_Layout.GetStride();
  }

  virtual ~GLVertexBuffer() { glDeleteBuffers(1, &m_RendererID); }

  void Bind() const override { glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); }
  void Unbind() const override { glBindBuffer(GL_ARRAY_BUFFER, 0); }

  void SetData(const void *data, u32 size) override {
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
  }

  void SetLayout(const BufferLayout &layout) override { m_Layout = layout; }
  const BufferLayout &GetLayout() const override { return m_Layout; }

private:
  u32 m_RendererID, m_Size;
  BufferLayout m_Layout;
};
}; // namespace Jupiter::Renderer
