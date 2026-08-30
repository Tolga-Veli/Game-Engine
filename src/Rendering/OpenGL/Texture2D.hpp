#pragma once

#include "Rendering/Backend/IFramebuffer.hpp"
#include "Rendering/Backend/ITexture.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
class Texture2D final : public ITexture {
public:
  Texture2D(const void *pixels, u32 width, u32 height, u32 channels);
  ~Texture2D() override;

  Texture2D(const Texture2D &) = delete;
  Texture2D(Texture2D &&) = delete;
  Texture2D &operator=(const Texture2D &) = delete;
  Texture2D &operator=(Texture2D &&) = delete;

  [[nodiscard]] u32 GetWidth() const noexcept override { return m_Width; }
  [[nodiscard]] u32 GetHeight() const noexcept override { return m_Height; }
  [[nodiscard]] u32 GetRendererID() const noexcept { return m_RendererID; }

  void Bind(u32 slot = 0) const noexcept override {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
  }

  void Unbind() const noexcept override { glBindTexture(GL_TEXTURE_2D, 0); }

private:
  void Upload(const void *pixels, u32 width, u32 height, u32 channels);
  void AllocateStorage(FramebufferTextureFormat format, u32 width, u32 height);

  u32 m_RendererID{}, m_Width{}, m_Height{};
};
} // namespace Jupiter::Rendering::OpenGL
