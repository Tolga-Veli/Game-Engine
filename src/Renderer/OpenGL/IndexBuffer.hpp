#pragma once

#include "Core/Utils.hpp"
#include "Renderer/Backend/IIndexBuffer.hpp"
#include "OpenGL-Utils.hpp"

#include <glad/gl.h>

namespace Jupiter::Renderer {
class GLIndexBuffer : public IIndexBuffer {
public:
  GLIndexBuffer() = default;
  GLIndexBuffer(const u32 *indices, u32 count, BufferUsage usage)
      : m_Count(count) {
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), indices,
                 JupiterToOpenGLBufferUsage(usage));
  }

  virtual ~GLIndexBuffer() { glDeleteBuffers(1, &m_RendererID); }

  void Bind() const override {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
  }

  void Unbind() const override { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

  u32 GetCount() const override { return m_Count; }

private:
  u32 m_RendererID{}, m_Count{};
};
}; // namespace Jupiter::Renderer
