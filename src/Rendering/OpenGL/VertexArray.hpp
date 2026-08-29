#pragma once

#include "OpenGL-Utils.hpp"
#include "Rendering/Backend/IVertexArray.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Jupiter::Rendering::OpenGL {

class VertexArray final : public IVertexArray {
public:
  VertexArray() { glGenVertexArrays(1, &m_RendererID); }
  virtual ~VertexArray() { glDeleteVertexArrays(1, &m_RendererID); }

  void Bind() const override { glBindVertexArray(m_RendererID); }
  void Unbind() const override { glBindVertexArray(0); }

  void AddVertexBuffer(const Ref<IVertexBuffer> &vertexBuffer,
                       u32 divisor = 0) override;

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
}; // namespace Jupiter::Rendering::OpenGL
