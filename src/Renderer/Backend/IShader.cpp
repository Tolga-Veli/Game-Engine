#include "IShader.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/OpenGL/Shader.hpp"

#include <memory>

namespace Jupiter::Renderer {

Ref<IShader> IShader::Create(
    const std::array<std::string, EnumToSize(ShaderType::Count)> &shaderSrcs) {
  switch (Renderer::GetAPI()) {
  case RendererAPI::OpenGL:
    return std::make_shared<GLShader>(shaderSrcs);
  default:
    return nullptr;
  }
}
} // namespace Jupiter::Renderer
