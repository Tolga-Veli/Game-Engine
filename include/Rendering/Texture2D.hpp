#pragma once

#include "Rendering/Backend/ITexture.hpp"

namespace Jupiter::Assets {
class AssetManager;

}

namespace Jupiter::Rendering {
class Texture2D {
public:
  Texture2D() = delete;
  ~Texture2D() noexcept = default;

  Texture2D(const Texture2D &) = delete;
  Texture2D &operator=(const Texture2D &) = delete;
  Texture2D(Texture2D &&) noexcept = default;
  Texture2D &operator=(Texture2D &&) noexcept = default;

  [[nodiscard]]
  u32 GetWidth() const noexcept {
    return m_Impl->GetWidth();
  }

  [[nodiscard]]
  u32 GetHeight() const noexcept {
    return m_Impl->GetHeight();
  }

  [[nodiscard]]
  bool IsValid() const noexcept {
    return m_Impl != nullptr;
  }

private:
  Scope<RHI::ITexture> m_Impl;

private:
  friend class Assets::AssetManager;

  explicit Texture2D(Scope<RHI::ITexture> impl) : m_Impl(std::move(impl)) {}
};

} // namespace Jupiter::Rendering
