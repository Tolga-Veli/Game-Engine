#include "Shader.hpp"
#include "Rendering/Backend/IShader.hpp"

namespace Jupiter::Rendering {

std::string_view Shader::GetName() const noexcept { return m_Impl->GetName(); }

const std::vector<ShaderParameter> &Shader::GetInputs() const noexcept {
  return m_Impl->GetInputs();
}

} // namespace Jupiter::Rendering
