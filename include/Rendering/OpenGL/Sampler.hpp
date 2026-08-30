#pragma once
#include "Core/Assert.hpp"
#include "Rendering/Backend/ISampler.hpp"
#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

inline constexpr GLenum JupiterToOpenGLFilterMode(RHI::FilterMode f) {
  using enum RHI::FilterMode;
  switch (f) {
  case Linear:
    return GL_LINEAR;
  case Nearest:
    return GL_NEAREST;
  default:
    UNREACHABLE("Unknown filter mode");
    return 0;
  }
}

inline constexpr GLenum JupiterToOpenGLWrapMode(RHI::WrapMode w) {
  using enum RHI::WrapMode;
  switch (w) {
  case Repeat:
    return GL_REPEAT;
  case ClampToEdge:
    return GL_CLAMP_TO_EDGE;
  case MirroredRepeat:
    return GL_MIRRORED_REPEAT;
  case ClampToBorder:
    return GL_CLAMP_TO_BORDER;
  default:
    UNREACHABLE("Unknown wrap mode");
    return 0;
  }
}

class Sampler final : public RHI::ISampler {
public:
  explicit Sampler(const RHI::SamplerSpec &spec) noexcept;

  ~Sampler() noexcept override { glDeleteSamplers(1, &m_RendererID); }

  Sampler(const Sampler &) = delete;
  Sampler &operator=(const Sampler &) = delete;
  Sampler(Sampler &&) noexcept;
  Sampler &operator=(Sampler &&) noexcept;

  [[nodiscard]]
  u32 GetRendererID() const noexcept {
    return m_RendererID;
  }

  void Bind(u32 slot) const noexcept { glBindSampler(slot, m_RendererID); }

private:
  u32 m_RendererID{};
};
} // namespace Jupiter::Rendering::OpenGL
