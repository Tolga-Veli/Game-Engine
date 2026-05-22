#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "OpenGL-Utils.hpp"
#include "Renderer/Backend/IVertexArray.hpp"

namespace Jupiter::Renderer {

class GLVertexArray : public IVertexArray {
public:
  GLVertexArray() { glGenVertexArrays(1, &m_RendererID); }
  virtual ~GLVertexArray() { glDeleteVertexArrays(1, &m_RendererID); }

  void Bind() const override { glBindVertexArray(m_RendererID); }
  void Unbind() const override { glBindVertexArray(0); }

  void AddVertexBuffer(const Ref<IVertexBuffer> &vertexBuffer) override;
  void SetIndexBuffer(const Ref<IIndexBuffer> &indexBuffer) override;

  const std::vector<Ref<IVertexBuffer>> &GetVertexBuffers() const override {
    return m_VertexBuffers;
  }

  const Ref<IIndexBuffer> &GetIndexBuffer() const override {
    return m_IndexBuffer;
  }

private:
  u32 m_RendererID{};
  std::vector<Ref<IVertexBuffer>> m_VertexBuffers;
  Ref<IIndexBuffer> m_IndexBuffer;
};
}; // namespace Jupiter::Renderer
