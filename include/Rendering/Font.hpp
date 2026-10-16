#pragma once

#include "Core/Utils.hpp"

#include "Rendering/Backend/IFont.hpp"

#include <glm/glm.hpp>
#include <stb_truetype.h>

namespace Jupiter::Assets {
class AssetManager;
}

namespace Jupiter::Rendering {
class Font {
public:
  Font() = delete;
  ~Font() noexcept = default;

  Font(const Font &) = delete;
  Font &operator=(const Font &) = delete;
  Font(Font &&) noexcept = default;
  Font &operator=(Font &&) noexcept = default;

  [[nodiscard]] const RHI::IFont::Glyph &GetGlyph(unsigned char c) const { return m_Impl->GetGlyph(c); }

  [[nodiscard]] std::span<const std::byte> GetAtlasPixels() const { return m_Impl->GetAtlasPixels(); }

  [[nodiscard]] u32 GetAtlasWidth() const { return m_Impl->GetAtlasWidth(); }
  [[nodiscard]] u32 GetAtlasHeight() const { return m_Impl->GetAtlasHeight(); }
  [[nodiscard]] float GetLineHeight() const { return m_Impl->GetLineHeight(); }

private:
  Scope<RHI::IFont> m_Impl;

private:
  friend class Assets::AssetManager;

  explicit Font(Scope<RHI::IFont> impl) : m_Impl(std::move(impl)) {}
};
} // namespace Jupiter::Rendering
