#pragma once

#include "Assets/AssetTraits.hpp"
#include "Assets/Parsers/MeshParser.hpp"

#include "Rendering/Mesh.hpp"

namespace Jupiter::Assets {
template <> struct AssetTraits<Rendering::Mesh> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Mesh;
  using Impl = Rendering::RHI::IMesh;
  using LoadOptions = std::monostate;
  using AssetData = Rendering::RHI::MeshDesc;

  static constexpr AssetType type = AssetType::Mesh;

  [[nodiscard]] static AssetData LoadDataFromFile(const fs::path &path, const LoadOptions &) {
    return MeshParser::Parse(path);
  }

  [[nodiscard]] static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  };
};

} // namespace Jupiter::Assets
