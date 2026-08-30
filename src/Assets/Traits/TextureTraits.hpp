#pragma once

#include "Core/Logging.hpp"

#include "Assets/AssetTraits.hpp"
#include "Rendering/Texture.hpp"

#include <stb_image.h>

namespace Jupiter::Assets {

template <> struct AssetTraits<Rendering::Texture> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Texture;
  using Impl = Rendering::ITexture;
  using LoadOptions = std::monostate;

  static constexpr AssetType type = AssetType::Texture;

  [[nodiscard]]
  static Scope<Impl> Create(const void *pixels, i32 width, i32 height,
                            i32 channels) {
    return Impl::Create(pixels, width, height, channels);
  }

  [[nodiscard]]
  static Scope<Impl> LoadFromFile(const fs::path &path, const LoadOptions &) {
    i32 width{}, height{}, channels{};
    stbi_set_flip_vertically_on_load(true);
    stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
      LOG_ERROR("Failed to load texture '{}' : {}", path.string(),
                stbi_failure_reason());
      return nullptr;
    }

    auto texture = Create(data, width, height, channels);
    stbi_image_free(data);
    return texture;
  }

  [[nodiscard]]
  static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  };
};

} // namespace Jupiter::Assets
