#pragma once

#include "Rendering/Backend/IFramebuffer.hpp"
#include "Rendering/Backend/ITexture.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
class Texture2D final : public ITexture {
public:
  Texture2D() = default;
  Texture2D(const void *pixels, u32 width, u32 height, u32 channels);

  static Ref<Texture2D> Create(const void *pixels, u32 width, u32 height,
                               u32 channels);

  static Ref<Texture2D> Create(FramebufferTextureFormat format, u32 width,
                               u32 height);

  void Bind(u32 slot = 0) const override {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
  }

  void Unbind() const override { glBindTexture(GL_TEXTURE_2D, 0); }

  u32 GetWidth() const noexcept override { return m_Width; }
  u32 GetHeight() const noexcept override { return m_Height; }
  u32 GetRendererID() const noexcept { return m_RendererID; }

private:
  void Upload(const void *pixels, u32 width, u32 height, u32 channels);
  void AllocateStorage(FramebufferTextureFormat format, u32 width, u32 height);

  u32 m_RendererID{}, m_Width{}, m_Height{};
};
} // namespace Jupiter::Rendering::OpenGL
