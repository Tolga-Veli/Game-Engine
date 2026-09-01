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

private:
  Scope<RHI::IFont> m_Impl;

private:
  friend class Assets::AssetManager;

  explicit Font(Scope<RHI::IFont> impl) : m_Impl(std::move(impl)) {}
};
} // namespace Jupiter::Rendering
