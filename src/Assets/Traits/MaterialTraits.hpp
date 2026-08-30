#pragma once

#include "Assets/AssetTraits.hpp"
#include "Rendering/Material.hpp"

namespace Jupiter::Assets {
template <> struct AssetTraits<Rendering::Material> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Material;
  using Impl = Rendering::IMaterial;
  using LoadOptions = std::monostate;

  static constexpr AssetType type = AssetType::Material;

  [[nodiscard]]
  static Scope<Impl>
  Create(Assets::AssetHandle<Rendering::Shader> shader,
         const std::unordered_map<std::string, Rendering::ShaderParamValue>
             &properties) {
    auto impl = Impl::Create();

    if (!impl)
      return nullptr;

    impl->SetShader(shader);

    for (const auto &[name, value] : properties)
      impl->SetProperty(name, value);

    return impl;
  }

  [[nodiscard]]
  static Scope<Impl> LoadFromFile(const fs::path &, const LoadOptions &) {
    // TODO:
    UNREACHABLE("TODO:");
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  }
};

} // namespace Jupiter::Assets
