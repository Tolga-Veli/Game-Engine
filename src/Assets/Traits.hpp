#pragma once

#include "Types.hpp"

#include "Rendering/Backend/ITexture.hpp"
#include "Rendering/Backend/IShader.hpp"

#include "Rendering/Mesh.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Font.hpp"

#include <filesystem>
#include <variant>

namespace Jupiter::Assets {
namespace fs = std::filesystem;

template <typename> inline constexpr bool dependent_false = false;

template <class T> struct AssetTraits {
  static_assert(dependent_false<T>,
                "AssetTraits hasn't been implemented for this AssetType");
};

struct FontLoadOptions {
  float size;
};

struct MeshLoadOptions {
  float scale = 1.0f;
};

namespace detail {
std::string NormalizePath(const fs::path &path) {
  return path.lexically_normal().generic_string();
}
} // namespace detail

using LoadOptions =
    std::variant<std::monostate, FontLoadOptions, MeshLoadOptions>;

template <> struct AssetTraits<Rendering::ITexture> {
  static constexpr AssetType Type = AssetType::Texture;
  static constexpr std::string_view Str = "Texture";

  static Ref<Rendering::ITexture> LoadFromFile(const fs::path &path,
                                               const LoadOptions &) {
    return Rendering::ITexture::LoadFromFile(path);
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), Str);
  };
};

template <> struct AssetTraits<Rendering::IShader> {
  static constexpr AssetType Type = AssetType::Shader;
  static constexpr std::string_view Str = "Shader";

  static Ref<Rendering::IShader> LoadFromFile(const fs::path &path,
                                              const LoadOptions &) {
    return Rendering::IShader::LoadFromFile(path);
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), Str);
  };
};

template <> struct AssetTraits<Rendering::Mesh> {
  static constexpr AssetType Type = AssetType::Mesh;
  static constexpr std::string_view Str = "Mesh";

  static Ref<Rendering::Mesh> LoadFromFile(const fs::path &path,
                                           const LoadOptions &opts) {
    [[maybe_unused]] auto mesh_opts = std::get<MeshLoadOptions>(opts);
    return Rendering::Mesh::LoadFromFile(path);
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &opts) {
    [[maybe_unused]] auto mesh_opts = std::get<MeshLoadOptions>(opts);
    return HashAssetKey(detail::NormalizePath(path), Str, mesh_opts.scale);
  };
};

template <> struct AssetTraits<Rendering::Font> {
  static constexpr AssetType Type = AssetType::Font;
  static constexpr std::string_view Str = "Font";

  static Ref<Rendering::Font> LoadFromFile(const fs::path &path,
                                           const LoadOptions &opts) {
    auto font_opts = std::get<FontLoadOptions>(opts);
    return Rendering::Font::LoadFromFile(path, font_opts.size);
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &opts) {
    auto font_opts = std::get<FontLoadOptions>(opts);
    return HashAssetKey(detail::NormalizePath(path), Str, font_opts.size);
  }
};

template <> struct AssetTraits<Rendering::Material> {
  static constexpr AssetType Type = AssetType::Material;
  static constexpr std::string_view Str = "Material";

  static Ref<Rendering::Material> LoadFromFile(const fs::path &path) {
    return Rendering::Material::LoadFromFile(path);
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), Str);
  }
};

} // namespace Jupiter::Assets
