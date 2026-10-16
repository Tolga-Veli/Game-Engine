#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Assets {
using AssetID = u64;

namespace detail {

constexpr u64 HashAssetName(std::string_view name) {
  // FNV-1a 64-bit offset basis and prime
  // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
  // (Collisions should be basically impossible)
  constexpr u64 offset_basis = 14695981039346656037ull, prime = 1099511628211ull;

  u64 hash = offset_basis;
  for (unsigned char c : name) {
    hash ^= static_cast<u64>(c);
    hash *= prime;
  }

  return hash;
}

template <class T>
concept ToStringCompatible = requires(T &&value) { std::to_string(std::forward<T>(value)); };

} // namespace detail

enum class AssetType : u8 { Texture, Shader, Audio, Font, Mesh, Material };

inline constexpr std::string_view to_string(AssetType type) {
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

template <detail::ToStringCompatible... Args>
inline constexpr u64 HashAssetKey(std::string_view path, AssetType type, Args &&...args) {
  std::string key = "file:";
  key += path;
  key += ':';
  key += to_string(type);

  [[maybe_unused]] auto append = [&](const auto &value) {
    key += ':';
    key += std::to_string(value);
  };

  (append(std::forward<Args>(args)), ...);

  return detail::HashAssetName(key);
}

inline u64 HashRuntimeAssetName(std::string_view name, AssetType type) {
  std::string key = "runtime:";
  key += to_string(type);
  key += ':';
  key += name;

  return detail::HashAssetName(key);
}

} // namespace Jupiter::Assets
