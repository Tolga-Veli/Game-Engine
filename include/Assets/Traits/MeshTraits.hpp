#pragma once

#include "Core/Assert.hpp"
#include "Assets/AssetTraits.hpp"
#include "Rendering/Mesh.hpp"

#include <stb_image.h>

namespace Jupiter::Assets {
template <> struct AssetTraits<Rendering::Mesh> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Mesh;
  using Impl = Rendering::RHI::IMesh;
  using LoadOptions = MeshLoadOptions;

  static constexpr AssetType type = AssetType::Mesh;

  template <class T>
  [[nodiscard]] static Scope<Impl>
  Create(std::span<const T> vertices, std::span<const u32> indices,
         const Rendering::RHI::BufferLayout &layout) {
    return Impl::Create(vertices, indices, layout);
  }

  [[nodiscard]] static Scope<Impl> LoadFromFile(const fs::path &,
                                                const LoadOptions &) {
    // TODO:
    UNREACHABLE("TODO:");
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &opts) {
    return HashAssetKey(detail::NormalizePath(path), type, opts.scale);
  };
};

} // namespace Jupiter::Assets
