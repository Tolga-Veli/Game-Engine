#include "Rendering/Backend/IShader.hpp"
#include "Rendering/Renderer.hpp"

#include "Core/Assert.hpp"

#include "Rendering/OpenGL/Shader.hpp"

namespace Jupiter::Rendering::RHI {
Scope<IShader> IShader::Create(
    const std::array<std::string, EnumToSize(ShaderType::Count)> &shaderSrcs) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return CreateScope<OpenGL::Shader>(shaderSrcs);
  default:
    UNREACHABLE("Unsupported renderer API.");
    return {};
  }
}

} // namespace Jupiter::Rendering::RHI
