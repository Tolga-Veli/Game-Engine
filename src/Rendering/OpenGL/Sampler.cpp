#include "Rendering/OpenGL/Sampler.hpp"
#include "Rendering/RendererAPI.hpp"

#include <utility>

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

Sampler::Sampler(Sampler &&other) noexcept : m_RendererID(std::exchange(other.m_RendererID, 0)) {}

Sampler &Sampler::operator=(Sampler &&other) noexcept {
  if (this != &other) {
    if (m_RendererID)
      glDeleteSamplers(1, &m_RendererID);
    m_RendererID = std::exchange(other.m_RendererID, 0);
  }
  return *this;
}
} // namespace Jupiter::Rendering::OpenGL

namespace Jupiter::Rendering::RHI {
Scope<ISampler> ISampler::Create(const SamplerSpec &spec) {
  if (RendererAPI::GetAPI() == GraphicsAPI::OpenGL)
    return CreateScope<OpenGL::Sampler>(spec);
  return {};
}
} // namespace Jupiter::Rendering::RHI
