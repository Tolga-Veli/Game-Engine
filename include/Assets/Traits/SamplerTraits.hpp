#pragma once

#include "Assets/AssetTraits.hpp"
#include "Assets/Types.hpp"
#include "Rendering/Sampler.hpp"

#include <stdexcept>

namespace Jupiter::Assets {
template <> struct AssetTraits<Rendering::Sampler> {
  static constexpr bool IsAsset = true;
  static constexpr AssetType type = AssetType::Sampler;

  using Impl = Rendering::RHI::ISampler;
  using LoadOptions = std::monostate;
  using AssetData = Rendering::RHI::SamplerSpec;

  [[nodiscard]] static AssetData LoadDataFromFile(const fs::path &, const LoadOptions &) {
    throw std::invalid_argument("Samplers must be created at runtime");
  }

  [[nodiscard]] static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  }
};
} // namespace Jupiter::Assets
