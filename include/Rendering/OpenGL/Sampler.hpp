// OpenGL/Sampler.hpp
#pragma once
#include "Rendering/Backend/ISampler.hpp"
#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

static GLenum ToGLFilter(FilterMode f) {
  return f == FilterMode::Linear ? GL_LINEAR : GL_NEAREST;
}

static GLenum ToGLWrap(WrapMode w) {
  switch (w) {
  case WrapMode::ClampToEdge:
    return GL_CLAMP_TO_EDGE;
  case WrapMode::MirroredRepeat:
    return GL_MIRRORED_REPEAT;
  default:
    return GL_REPEAT;
  }
}

class Sampler final : public ISampler {
public:
  explicit Sampler(const SamplerSpec &spec) {
    glCreateSamplers(1, &m_RendererID);
    glSamplerParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER,
                        ToGLFilter(spec.minFilter));
    glSamplerParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER,
                        ToGLFilter(spec.magFilter));
    glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_S, ToGLWrap(spec.wrapS));
    glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_T, ToGLWrap(spec.wrapT));
  }

  ~Sampler() override { glDeleteSamplers(1, &m_RendererID); }

  // Bound to a texture UNIT, independently of whatever texture is bound
  // to that same unit - this is the whole point: texture data and sampling
  // behavior are now two orthogonal bind calls instead of one coupled object.
  void Bind(u32 slot) const override { glBindSampler(slot, m_RendererID); }

private:
  u32 m_RendererID{};
};
} // namespace Jupiter::Rendering::OpenGL
