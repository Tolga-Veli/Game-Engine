#pragma once

#include "Assets/AssetTraits.hpp"
#include "Assets/Parsers/FontParser.hpp"

#include "Rendering/Font.hpp"

namespace Jupiter::Assets {
template <> struct AssetTraits<Rendering::Font> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Font;
  using Impl = Rendering::RHI::IFont;
  using LoadOptions = Rendering::RHI::FontOptions;
  using AssetData = Rendering::RHI::FontData;

  static constexpr AssetType type = AssetType::Font;

  [[nodiscard]] static AssetData LoadDataFromFile(const fs::path &path, const LoadOptions &opts) {
    return FontParser::Parse(path, opts);
  }

  [[nodiscard]] static AssetID MakeID(const fs::path &path, const LoadOptions &opts) {
    return HashAssetKey(detail::NormalizePath(path), type, opts.size);
  }
};

} // namespace Jupiter::Assets
