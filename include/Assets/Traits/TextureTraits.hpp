#pragma once

#include "Core/Logging.hpp"

#include "Assets/AssetTraits.hpp"
#include "Rendering/Backend/ITexture.hpp"
#include "Rendering/Texture2D.hpp"

#include <cstddef>
#include <stb_image.h>

namespace Jupiter::Assets {

template <> struct AssetTraits<Rendering::Texture2D> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Texture2D;
  using Impl = Rendering::RHI::ITexture;
  using LoadOptions = std::monostate;

  static constexpr AssetType type = AssetType::Texture;

  [[nodiscard]]
  static Scope<Impl> Create(const Rendering::RHI::TextureCreateSpec &spec) {
    return Impl::Create(spec);
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

    const std::size_t sz = static_cast<std::size_t>(width) * height * channels;
    const auto format = Rendering::RHI::ToTextureFormat(channels);

    if (format == Rendering::RHI::TextureFormat::None) {
      LOG_ERROR("Texture format was None");
      stbi_image_free(data);
      return nullptr;
    }

    auto texture = Create(Rendering::RHI::TextureCreateSpec{
        .data =
            std::span<const std::byte>{
                reinterpret_cast<const std::byte *>(data), sz},
        .width = static_cast<u32>(width),
        .height = static_cast<u32>(height),
        .format = format});

    stbi_image_free(data);
    return texture;
  }

  [[nodiscard]]
  static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  };
};

} // namespace Jupiter::Assets
