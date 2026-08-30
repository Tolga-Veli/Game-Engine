#include "Rendering/OpenGL/Sampler.hpp"

namespace Jupiter::Rendering::OpenGL {
Sampler::Sampler(const RHI::SamplerSpec &info) noexcept {
  glCreateSamplers(1, &m_RendererID);

  glSamplerParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER,
                      JupiterToOpenGLFilterMode(info.minFilter));

  glSamplerParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER,
                      JupiterToOpenGLFilterMode(info.magFilter));

  glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_S,
                      JupiterToOpenGLWrapMode(info.wrapS));

  glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_T,
                      JupiterToOpenGLWrapMode(info.wrapT));
}
} // namespace Jupiter::Rendering::OpenGL
