#pragma once

#include "Core/Logging.hpp"

#include "Assets/Types.hpp"
#include "Assets/AssetTraits.hpp"
#include "Data/FileStream.hpp"
#include "Rendering/Backend/ITexture.hpp"
#include "Rendering/Texture2D.hpp"

#include <cstddef>
#include <limits>
#include <memory>
#include <variant>
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
    const std::string fileData = Data::ReadFile(path);
    if (fileData.empty())
      return nullptr;

    if (fileData.size() >
        static_cast<std::size_t>(std::numeric_limits<int>::max())) {
      LOG_ERROR("Texture '{}' is too large for stb_image", path.string());
      return nullptr;
    }

    i32 width{}, height{}, channels{};
    stbi_set_flip_vertically_on_load(true);
    using ImageData = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
    ImageData data{stbi_load_from_memory(
                       reinterpret_cast<const stbi_uc *>(fileData.data()),
                       static_cast<int>(fileData.size()), &width, &height,
                       &channels, 0),
                   &stbi_image_free};

    if (!data) {
      LOG_ERROR("Failed to load texture '{}' : {}", path.string(),
                stbi_failure_reason());
      return nullptr;
    }

    if (width <= 0 || height <= 0 || channels <= 0) {
      LOG_ERROR("Texture '{}' has invalid dimensions ({}x{}, {} channels)",
                path.string(), width, height, channels);
      return nullptr;
    }

    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);
    const auto c = static_cast<std::size_t>(channels);
    constexpr auto maxSize = std::numeric_limits<std::size_t>::max();
    if (w > maxSize / h || w * h > maxSize / c) {
      LOG_ERROR("Texture '{}' dimensions overflow addressable memory",
                path.string());
      return nullptr;
    }

    const std::size_t sz = w * h * c;
    const auto format = Rendering::RHI::ToTextureFormat(channels);

    if (format == Rendering::RHI::TextureFormat::None) {
      LOG_ERROR("Texture format was None");
      return nullptr;
    }

    auto texture = Create(Rendering::RHI::TextureCreateSpec{
        .data =
            std::span<const std::byte>{
                reinterpret_cast<const std::byte *>(data.get()), sz},
        .width = static_cast<u32>(width),
        .height = static_cast<u32>(height),
        .format = format});

    return texture;
  }

  [[nodiscard]]
  static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  };
};

} // namespace Jupiter::Assets
