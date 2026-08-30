#pragma once

#include "Rendering/Backend/ITexture.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
class Texture2D final : public RHI::ITexture {
public:
  explicit Texture2D(const RHI::TextureCreateSpec &info) noexcept;
  ~Texture2D() noexcept override;

  Texture2D(const Texture2D &) = delete;
  Texture2D &operator=(const Texture2D &) = delete;
  Texture2D(Texture2D &&other) noexcept;
  Texture2D &operator=(Texture2D &&other) noexcept;

  [[nodiscard]] u32 GetWidth() const noexcept override { return m_Width; }
  [[nodiscard]] u32 GetHeight() const noexcept override { return m_Height; }
  [[nodiscard]] u32 GetRendererID() const noexcept { return m_RendererID; }

  void Bind(u32 slot = 0) const noexcept;

private:
  void Upload(const void *pixels, u32 width, u32 height, u32 channels);

  u32 m_RendererID{}, m_Width{}, m_Height{};
};
} // namespace Jupiter::Rendering::OpenGL
