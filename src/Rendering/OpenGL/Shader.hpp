#pragma once

#include "Rendering/Backend/IShader.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

class Shader final : public IShader {
public:
  explicit Shader(
      const std::array<std::string, EnumToSize(ShaderType::Count)> &shaderSrcs);

  ~Shader() override;

  [[nodiscard]]
  std::string_view GetName() const noexcept override {
    return m_Name;
  }

  [[nodiscard]]
  const std::vector<ShaderParameter> &GetInputs() const noexcept override {
    return m_Inputs;
  }

  [[nodiscard]]
  u32 GetProgram() const noexcept {
    return m_RendererID;
  }

private:
  u32 m_RendererID = 0;
  std::string m_Name;
  std::vector<ShaderParameter> m_Inputs;

  void Compile(ShaderType type, std::string_view source);
  void Reflect();
};

} // namespace Jupiter::Rendering::OpenGL
