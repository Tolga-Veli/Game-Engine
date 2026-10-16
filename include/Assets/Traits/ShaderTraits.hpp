#pragma once

#include "Assets/AssetTraits.hpp"
#include "Assets/Parsers/ShaderParser.hpp"

#include "Rendering/Shader.hpp"

namespace Jupiter::Assets {
template <> struct AssetTraits<Rendering::Shader> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Shader;
  using Impl = Rendering::RHI::IShader;
  using LoadOptions = std::monostate;
  using AssetData = Rendering::RHI::ShaderSources;

  static constexpr AssetType type = AssetType::Shader;

  [[nodiscard]] static AssetData LoadDataFromFile(const fs::path &path, const LoadOptions &) {
    return ShaderParser::Parse(path);
  }

  [[nodiscard]] static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  };
};
} // namespace Jupiter::Assets
