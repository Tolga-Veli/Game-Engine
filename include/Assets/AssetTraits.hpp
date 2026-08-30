#pragma once

#include "Types.hpp"

#include <concepts>
#include <filesystem>
#include <variant>

namespace Jupiter::Assets {
namespace fs = std::filesystem;

template <class T> struct AssetTraits {
  static constexpr bool IsAsset = false;
};

struct FontLoadOptions {
  float size = 16.0f;
};

struct MeshLoadOptions {
  float scale = 1.0f;
};

using LoadOptions =
    std::variant<std::monostate, FontLoadOptions, MeshLoadOptions>;

template <class T>
concept Asset = requires {
  { AssetTraits<T>::IsAsset } -> std::convertible_to<bool>;
  AssetTraits<T>::type;
  typename AssetTraits<T>::LoadOptions;
} && AssetTraits<T>::IsAsset;

namespace detail {
inline std::string NormalizePath(const fs::path &path) {
  return path.lexically_normal().generic_string();
}
} // namespace detail
} // namespace Jupiter::Assets
