#pragma once

#include "Core/Assert.hpp"
#include "Assets/AssetTraits.hpp"
#include "Rendering/Mesh.hpp"

#include <stb_image.h>

namespace Jupiter::Assets {
template <> struct AssetTraits<Rendering::Mesh> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Mesh;
  using Impl = Rendering::IMesh;
  using LoadOptions = MeshLoadOptions;

  static constexpr AssetType type = AssetType::Mesh;

  [[nodiscard]] static Scope<Impl> Create(const void *vertices, u32 vertexCount,
                                          u32 vertexStride, const void *indices,
                                          u32 indexCount) {
    return Impl::Create(vertices, vertexCount, vertexStride, indices,
                        indexCount);
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
