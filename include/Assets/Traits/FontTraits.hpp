#pragma once

#include "Assets/Types.hpp"
#include "Assets/AssetTraits.hpp"

#include "Rendering/Font.hpp"
#include "Data/FileStream.hpp"

namespace Jupiter::Assets {

template <> struct AssetTraits<Rendering::Font> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Font;
  using Impl = Rendering::RHI::IFont;
  using LoadOptions = FontLoadOptions;

  static constexpr AssetType type = AssetType::Font;

  [[nodiscard]]
  static Scope<Impl> Create(const void *data, std::size_t fontSize,
                            float pixelHeight) {
    return Impl::Create(data, fontSize, pixelHeight);
  }

  [[nodiscard]] static Scope<Impl> LoadFromFile(const fs::path &path,
                                                const LoadOptions &opts) {
    const std::string data = Data::ReadFile(path);
    if (data.empty())
      return nullptr;
    return Create(data.data(), data.size(), opts.size);
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &opts) {
    return HashAssetKey(detail::NormalizePath(path), type, opts.size);
  }
};

} // namespace Jupiter::Assets
