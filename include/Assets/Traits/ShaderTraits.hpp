#pragma once

#include "Core/Logging.hpp"
#include "Data/FileStream.hpp"

#include "Assets/AssetTraits.hpp"
#include "Rendering/Shader.hpp"
#include <variant>

namespace Jupiter::Assets {

namespace detail {

inline Rendering::ShaderType ShaderTypeFromString(std::string_view type) {
  if (type == "vertex")
    return Rendering::ShaderType::Vertex;
  if (type == "fragment")
    return Rendering::ShaderType::Fragment;
  if (type == "geometry")
    return Rendering::ShaderType::Geometry;
  if (type == "compute")
    return Rendering::ShaderType::Compute;
  return Rendering::ShaderType::None;
}
} // namespace detail

template <> struct AssetTraits<Rendering::Shader> {
  static constexpr bool IsAsset = true;

  using Asset = Rendering::Shader;
  using Impl = Rendering::IShader;
  using LoadOptions = std::monostate;

  static constexpr AssetType type = AssetType::Shader;

  using ShaderSources =
      std::array<std::string, EnumToSize(Rendering::ShaderType::Count)>;

  [[nodiscard]]
  static Scope<Impl> Create(const ShaderSources &sources) {
    return Impl::Create(sources);
  }

  [[nodiscard]] static Scope<Impl> LoadFromFile(const fs::path &path,
                                                const LoadOptions &) {
    ShaderSources sources{};

    const std::string source = Data::ReadFile(path);
    constexpr std::string_view token = "#type";

    std::size_t pos = source.find(token);
    while (pos != std::string::npos) {
      const std::size_t eol = source.find_first_of("\r\n", pos);
      if (eol == std::string::npos) {
        LOG_ERROR("Malformed shader '{}' - missing new line after #type",
                  path.string());
        return nullptr;
      }

      const std::size_t typeBegin = pos + token.size(),
                        typeStart = source.find_first_not_of(" \t", typeBegin);

      if (typeStart == std::string::npos || typeStart >= eol) {
        LOG_ERROR("Malformed shader '{}' - missing shader type", path.string());
        return nullptr;
      }

      const std::string_view typeName(source.data() + typeStart,
                                      eol - typeStart);
      const Rendering::ShaderType type = detail::ShaderTypeFromString(typeName);
      if (type == Rendering::ShaderType::None) {
        LOG_ERROR("Unknown shader type '{}' in '{}'", typeName, path.string());
        return nullptr;
      }

      const std::size_t sourceBegin = source.find_first_not_of("\r\n", eol);
      if (sourceBegin == std::string::npos) {
        LOG_ERROR("Missing source for shader type '{}' in '{}'", typeName,
                  path.string());
        return nullptr;
      }

      const std::size_t next = source.find(token, sourceBegin),
                        sourceEnd =
                            next == std::string::npos ? source.size() : next;

      sources[static_cast<std::size_t>(type)] =
          source.substr(sourceBegin, sourceEnd - sourceBegin);
      pos = next;
    }
    return Create(sources);
  }

  static AssetID MakeID(const fs::path &path, const LoadOptions &) {
    return HashAssetKey(detail::NormalizePath(path), type);
  };
};
} // namespace Jupiter::Assets
