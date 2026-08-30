#include "Rendering/OpenGL/Texture2D.hpp"

#include "Core/Assert.hpp"
#include "Rendering/Backend/ITexture.hpp"
#include "Rendering/OpenGL/OpenGL-Utils.hpp"

#include <utility>

namespace Jupiter::Rendering::OpenGL {

Texture2D::Texture2D(const RHI::TextureCreateSpec &info) noexcept
    : m_Width(info.width), m_Height(info.height) {

  CORE_ASSERT(m_Width > 0, "Texture width cannot be zero");
  CORE_ASSERT(m_Height > 0, "Texture height cannot be zero");

  glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

  glTextureStorage2D(m_RendererID, 1,
                     JupiterToOpenGLInternalFormat(info.format), info.width,
                     info.height);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTextureSubImage2D(m_RendererID, 0, 0, 0, info.width, info.height,
                      JupiterToOpenGLDataFormat(info.format), GL_UNSIGNED_BYTE,
                      info.data.data());
}

Texture2D::~Texture2D() noexcept {
  if (m_RendererID)
    glDeleteTextures(1, &m_RendererID);
}

Texture2D::Texture2D(Texture2D &&other) noexcept
    : m_RendererID(std::exchange(other.m_RendererID, 0)),
      m_Width(std::exchange(other.m_Width, 0)),
      m_Height(std::exchange(other.m_Height, 0)) {}

Texture2D &Texture2D::operator=(Texture2D &&other) noexcept {
  if (this == &other)
    return *this;

  if (m_RendererID)
    glDeleteTextures(1, &m_RendererID);

  m_RendererID = std::exchange(other.m_RendererID, 0);
  m_Width = std::exchange(other.m_Width, 0);
  m_Height = std::exchange(other.m_Height, 0);

  return *this;
}

void Texture2D::Bind(u32 slot) const noexcept {
  glBindTextureUnit(slot, m_RendererID);
}

} // namespace Jupiter::Rendering::OpenGL
