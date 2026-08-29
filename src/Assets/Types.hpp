#pragma once

#include "Core/Utils.hpp"

#include <limits>

namespace Jupiter::Assets {

using AssetID = u64;

namespace detail {

constexpr AssetID HashAssetName(std::string_view name) {
  // FNV-1a 64-bit offset basis and prime
  // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
  // (Collisions should be basically impossible)
  constexpr u64 offset_basis = 14695981039346656037ull,
                prime = 1099511628211ull;

  u64 hash = offset_basis;
  for (unsigned char c : name) {
    hash ^= static_cast<u64>(c);
    hash *= prime;
  }

  return hash;
}

template <class T>
concept ToStringCompatible =
    requires(T &&value) { std::to_string(std::forward<T>(value)); };

} // namespace detail

template <detail::ToStringCompatible... Args>
inline constexpr AssetID HashAssetKey(std::string_view path,
                                      std::string_view type, Args &&...args) {
  std::string key = "file:";
  key += path;
  key += ':';
  key += type;

  auto append = [&](const auto &value) {
    key += ':';
    key += std::to_string(value);
  };

  (append(std::forward<Args>(args)), ...);

  return detail::HashAssetName(key);
}

enum class AssetType : u8 { Texture, Shader, Audio, Font, Mesh, Material };

inline std::string_view to_string(AssetType type) {
  switch (type) {
  case AssetType::Texture:
    return "Texture";
  case AssetType::Shader:
    return "Shader";
  case AssetType::Audio:
    return "Audio";
  case AssetType::Font:
    return "Font";
  case AssetType::Mesh:
    return "Mesh";
  case AssetType::Material:
    return "Material";
  }

  return "Unknown";
}

inline AssetID HashRuntimeAssetName(std::string_view name, AssetType type) {
  std::string key = "runtime:";
  key += to_string(type);
  key += ':';
  key += name;

  return detail::HashAssetName(key);
}

inline constexpr u32 k_InvalidAssetIndex = std::numeric_limits<u32>::max();

template <class T> struct AssetHandle {
  u32 idx{k_InvalidAssetIndex}, gen{};

  constexpr bool IsValid() const noexcept { return idx != k_InvalidAssetIndex; }
  constexpr bool operator==(const AssetHandle &other) const noexcept = default;
};

} // namespace Jupiter::Assets
