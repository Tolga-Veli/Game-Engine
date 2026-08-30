#pragma once

#include "Rendering/Backend/ITexture.hpp"

namespace Jupiter::Assets {
class AssetManager;

}

namespace Jupiter::Rendering {

class Texture {
public:
  ~Texture() = default;

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;
  Texture(Texture &&) noexcept = default;
  Texture &operator=(Texture &&) noexcept = default;

  [[nodiscard]]
  u32 GetWidth() const noexcept {
    return m_Impl->GetWidth();
  }

  [[nodiscard]]
  u32 GetHeight() const noexcept {
    return m_Impl->GetHeight();
  }

  void Bind(u32 slot = 0) const noexcept { m_Impl->Bind(slot); }
  void Unbind() const noexcept { m_Impl->Unbind(); }

  [[nodiscard]]
  bool IsValid() const noexcept {
    return m_Impl != nullptr;
  }

private:
  Scope<ITexture> m_Impl;

private:
  friend class Assets::AssetManager;

  explicit Texture(Scope<ITexture> impl) : m_Impl(std::move(impl)) {}
};

} // namespace Jupiter::Rendering
