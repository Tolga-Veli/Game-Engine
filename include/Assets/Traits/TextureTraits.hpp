#pragma once

#include "Assets/AssetTraits.hpp"
#include "Assets/Parsers/TextureParser.hpp"

#include "Rendering/Texture2D.hpp"

namespace Jupiter::Assets {
template <> struct AssetTraits<Rendering::Texture2D> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Texture2D;
  using Impl = Rendering::RHI::ITexture;
  using LoadOptions = std::monostate;
  using AssetData = Rendering::RHI::TextureData;

  static constexpr AssetType type = AssetType::Texture;

  [[nodiscard]] static AssetData LoadDataFromFile(const fs::path &path, const LoadOptions &) {
    return TextureParser::Parse(path);
  }

  [[nodiscard]] static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  };
};

} // namespace Jupiter::Assets
