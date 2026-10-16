#pragma once

#include "Assets/AssetTraits.hpp"
#include "Assets/Parsers/MaterialParser.hpp"

#include "Rendering/Material.hpp"

namespace Jupiter::Assets {
template <> struct AssetTraits<Rendering::Material> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Material;
  using Impl = Rendering::RHI::IMaterial;
  using LoadOptions = std::monostate;
  using AssetData = Rendering::RHI::MaterialData;

  static constexpr AssetType type = AssetType::Material;

  [[nodiscard]] static Rendering::RHI::MaterialData LoadDataFromFile(const fs::path &path, const LoadOptions &) {
    return MaterialParser::Parse(path);
  }

  [[nodiscard]] static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  }
};

} // namespace Jupiter::Assets
