#pragma once

#include "Core/Assert.hpp"
#include "Rendering/Backend/IShader.hpp"

#include <unordered_map>
#include <variant>

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

inline RHI::ShaderDataType OpenGLToJupiterShaderDataType(GLenum type) {
  switch (type) {
  case GL_FLOAT:
    return RHI::ShaderDataType::Float;

  case GL_FLOAT_VEC2:
    return RHI::ShaderDataType::Float2;

  case GL_FLOAT_VEC3:
    return RHI::ShaderDataType::Float3;

  case GL_FLOAT_VEC4:
    return RHI::ShaderDataType::Float4;

  case GL_INT:
    return RHI::ShaderDataType::Int;

  case GL_UNSIGNED_INT:
    return RHI::ShaderDataType::UInt;

  case GL_FLOAT_MAT3:
    return RHI::ShaderDataType::Mat3;

  case GL_FLOAT_MAT4:
    return RHI::ShaderDataType::Mat4;

  case GL_SAMPLER_2D:
  case GL_SAMPLER_CUBE:
  default:
    UNREACHABLE("Unsupported OpenGL shader data type");
    return RHI::ShaderDataType::None;
  }
}

class Shader final : public RHI::IShader {
public:
  explicit Shader(const ShaderSources &shaderSrcs);
  ~Shader() override;

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;
  Shader(Shader &&) = delete;
  Shader &operator=(Shader &&) = delete;

  [[nodiscard]]
  std::string_view GetName() const noexcept override {
    return m_Name;
  }

  [[nodiscard]]
  const std::vector<RHI::ShaderParameter> &GetInputs() const noexcept override {
    return m_Inputs;
  }

  void Set(std::string_view name, const RHI::ShaderParamValue &value) {

    std::visit([this, name](const auto &value) { Set(name, value); }, value);
  }

  void Bind() const noexcept { glUseProgram(m_RendererID); }

  void Set(std::string_view name, bool value) override;
  void Set(std::string_view name, i32 value) override;
  void Set(std::string_view name, u32 value) override;
  void Set(std::string_view name, f32 value) override;

  void Set(std::string_view name, const glm::vec2 &value) override;
  void Set(std::string_view name, const glm::vec3 &value) override;
  void Set(std::string_view name, const glm::vec4 &value) override;

  void Set(std::string_view name, const glm::mat3 &value) override;
  void Set(std::string_view name, const glm::mat4 &value) override;
  void Set(std::string_view name, const RHI::TextureBinding &binding) override;

  void ResetTextureSlots() noexcept { m_NextTextureSlot = 0; }

private:
  u32 m_RendererID{};
  std::string m_Name;

  std::vector<RHI::ShaderParameter> m_Inputs;

  std::unordered_map<std::string, i32> m_UniformLocCache;

  u32 m_NextTextureSlot{};

  void Compile(RHI::ShaderType type, std::string_view source);
  void Reflect();

  [[nodiscard]]
  i32 GetUniformLoc(std::string_view name);

  [[nodiscard]]
  u32 AllocateTextureSlot() noexcept {
    return m_NextTextureSlot++;
  }
};

} // namespace Jupiter::Rendering::OpenGL
