#pragma once

#include "Core/Assert.hpp"
#include "Assets/AssetTraits.hpp"
#include "Rendering/Font.hpp"

namespace Jupiter::Assets {

template <> struct AssetTraits<Rendering::Font> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Font;
  using Impl = Rendering::IFont;
  using LoadOptions = FontLoadOptions;

  static constexpr AssetType type = AssetType::Font;

  [[nodiscard]]
  static Scope<Impl> Create(const void *data, std::size_t fontSize,
                            float pixelHeight) {
    return Impl::Create(data, fontSize, pixelHeight);
  }

  [[nodiscard]] static Scope<Impl> LoadFromFile(const fs::path &,
                                                const LoadOptions &) {
    // TODO:
    UNREACHABLE("TODO:");
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &opts) {
    return HashAssetKey(detail::NormalizePath(path), type, opts.size);
  }
};

} // namespace Jupiter::Assets
