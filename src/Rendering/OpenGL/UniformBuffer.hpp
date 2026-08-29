#pragma once

#include "Rendering/Backend/IUniformBuffer.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
class UniformBuffer final : public IUniformBuffer {
public:
  UniformBuffer(u32 size, u32 binding) {
    glCreateBuffers(1, &m_RendererID);
    glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);

    // Binds the buffer to a biding point ONCE, at creation - this is what lets
    // shaders reference it by 'binding  = N' without any per-draw wiring,
    // unlike regular uniforms
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
  }

  ~UniformBuffer() override { glDeleteBuffers(1, &m_RendererID); }

  void SetData(const void *data, u32 size, u32 offset = 0) override {
    glNamedBufferSubData(m_RendererID, offset, size, data);
  }

private:
  u32 m_RendererID{};
};
} // namespace Jupiter::Rendering::OpenGL
