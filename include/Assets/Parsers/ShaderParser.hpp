#pragma once

#include "Rendering/Backend/IShader.hpp"
#include "Rendering/Backend/Render-Utils.hpp"

#include "Data/FileStream.hpp"

namespace Jupiter::Assets {

namespace detail {
inline Rendering::RHI::ShaderType ShaderTypeFromString(std::string_view type) {
  using enum Rendering::RHI::ShaderType;
  if (type == "vertex")
    return Vertex;
  if (type == "fragment")
    return Fragment;
  if (type == "geometry")
    return Geometry;
  if (type == "compute")
    return Compute;
  else
    return None;
}
} // namespace detail

class ShaderParser {
public:
  [[nodiscard]] static Rendering::RHI::ShaderSources Parse(const std::filesystem::path &path) {
    Rendering::RHI::ShaderSources sources{};

    Storage buffer;
    Data::ReadFile(path, buffer);
    std::string source(reinterpret_cast<const char *>(buffer.data()), buffer.size());

    constexpr std::string_view token = "#type";

    std::size_t pos = source.find(token);
    while (pos != std::string::npos) {
      const std::size_t eol = source.find_first_of("\r\n", pos);
      if (eol == std::string::npos) {
        LOG_ERROR("Malformed shader '{}' - missing new line after #type", path.string());
        return {};
      }

      const std::size_t typeBegin = pos + token.size(), typeStart = source.find_first_not_of(" \t", typeBegin);
      if (typeStart == std::string::npos || typeStart >= eol) {
        LOG_ERROR("Malformed shader '{}' - missing shader type", path.string());
        return {};
      }

      const std::string_view typeName(source.data() + typeStart, eol - typeStart);
      const Rendering::RHI::ShaderType type = detail::ShaderTypeFromString(typeName);
      if (type == Rendering::RHI::ShaderType::None) {
        LOG_ERROR("Unknown shader type '{}' in '{}'", typeName, path.string());
        return {};
      }

      const std::size_t sourceBegin = source.find_first_not_of("\r\n", eol);
      if (sourceBegin == std::string::npos) {
        LOG_ERROR("Missing source for shader type '{}' in '{}'", typeName, path.string());
        return {};
      }

      const std::size_t next = source.find(token, sourceBegin),
                        sourceEnd = next == std::string::npos ? source.size() : next;

      sources[static_cast<std::size_t>(type)] = source.substr(sourceBegin, sourceEnd - sourceBegin);
      pos = next;
    }
    return sources;
  }
};
} // namespace Jupiter::Assets
