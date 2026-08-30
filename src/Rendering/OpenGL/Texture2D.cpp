#include "Core/Logging.hpp"
#include "Rendering/Backend/IFramebuffer.hpp"
#include "Rendering/OpenGL/OpenGL-Utils.hpp"

#include "Texture2D.hpp"

#include <GL/gl.h>
#include <stb_image.h>
#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

Texture2D::Texture2D(const void *pixels, u32 width, u32 height, u32 channels)
    : m_Width(width), m_Height(height) {
  Upload(pixels, width, height, channels);
}

Texture2D::~Texture2D() {
  if (m_RendererID)
    glDeleteTextures(1, &m_RendererID);
}

void Texture2D::Upload(const void *pixels, u32 width, u32 height,
                       u32 channels) {
  CORE_ASSERT(pixels, "Failed to load texture!");

  GLenum internalFormat = 0, dataFormat = 0;
  switch (channels) {
  case 1:
    internalFormat = GL_R8;
    dataFormat = GL_RED;
    break;
  case 3:
    internalFormat = GL_RGB8;
    dataFormat = GL_RGB;
    break;
  case 4:
    internalFormat = GL_RGBA8;
    dataFormat = GL_RGBA;
    break;
  default:
    LOG_ERROR("GLTexture: unsupported channel count {}", channels);
    return;
  };

  glGenTextures(1, &m_RendererID);
  glBindTexture(GL_TEXTURE_2D, m_RendererID);

  // matters for the single-channel atlas case
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat,
               GL_UNSIGNED_BYTE, pixels);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::AllocateStorage(FramebufferTextureFormat format, u32 width,
                                u32 height) {
  m_Width = width;
  m_Height = height;

  glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
  glTextureStorage2D(m_RendererID, 1,
                     JupiterToOpenGLFramebufferTextureFormat(format), width,
                     height);

  // integer formats can't be linearly filtered — GL either errors or gives
  // undefined results depending on driver, so RedInteger must be NEAREST
  GLenum filter =
      (format == FramebufferTextureFormat::RedInteger) ? GL_NEAREST : GL_LINEAR;

  glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, filter);
  glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filter);
  glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

} // namespace Jupiter::Rendering::OpenGL
