#include "IShader.hpp"

#include "Data/FileStream.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/Shader.hpp"

#include <memory>
#include <string>

namespace Jupiter::Rendering {
Ref<IShader> IShader::Create(
    const std::array<std::string, EnumToSize(ShaderType::Count)> &shaderSrcs) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_shared<OpenGL::Shader>(shaderSrcs);
  default:
    LOG_ERROR("inacessible");
    return nullptr;
  }
}
Ref<IShader> IShader::LoadFromFile(const std::filesystem::path &path) {
  std::array<std::string, EnumToSize(ShaderType::Count)> shaderSrcs;

  static auto StringToType = [](const std::string &type) -> ShaderType {
    if (type == "vertex")
      return ShaderType::Vertex;
    else if (type == "fragment")
      return ShaderType::Fragment;
    else if (type == "geometry")
      return ShaderType::Geometry;
    else if (type == "compute")
      return ShaderType::Compute;
    return ShaderType::None;
  };

  std::string source = Data::ReadFile(path);

  std::string_view token = "#type";
  size_t pos = source.find(token, 0);
  while (pos != std::string::npos) {
    size_t eol = source.find_first_of("\r\n", pos);
    size_t beg = pos + token.length() + 1;
    std::string type = source.substr(beg, eol - beg);

    size_t nextLinePos = source.find_first_not_of("\r\n", eol);
    pos = source.find(token, nextLinePos);

    shaderSrcs[static_cast<u32>(StringToType(type))] =
        (pos == std::string::npos)
            ? source.substr(nextLinePos)
            : source.substr(nextLinePos, pos - nextLinePos);
  }
  return Create(shaderSrcs);
}
} // namespace Jupiter::Rendering
