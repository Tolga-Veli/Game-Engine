#pragma once

#include "Rendering/Backend/IShader.hpp"

#include <unordered_map>
#include <variant>

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

inline RHI::DataType OpenGLToJupiterDataType(GLenum type) {
  using enum RHI::DataType;
  switch (type) {
  case GL_FLOAT:
    return Float;
  case GL_FLOAT_VEC2:
    return Float2;
  case GL_FLOAT_VEC3:
    return Float3;
  case GL_FLOAT_VEC4:
    return Float4;

  case GL_INT:
    return Int;
  case GL_UNSIGNED_INT:
    return UInt;

  case GL_FLOAT_MAT3:
    return Mat3;
  case GL_FLOAT_MAT4:
    return Mat4;

  case GL_SAMPLER_2D:
    return Texture2D;

  case GL_SAMPLER_CUBE:
    // Cubemap resources are not represented by the RHI yet. Reflection must
    // not crash merely because a shader contains an unsupported uniform.
    return None;

  default:
    // Reflection is descriptive, not a validation step. Types that are not
    // exposed by ShaderParamValue are safely omitted from the material UI.
    return None;
  }
}

class Shader final : public RHI::IShader {
public:
  explicit Shader(const RHI::ShaderSources &shaderSrcs);
  ~Shader() override;

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;
  Shader(Shader &&) = delete;
  Shader &operator=(Shader &&) = delete;

  [[nodiscard]] std::string_view GetName() const noexcept override { return m_Name; }
  [[nodiscard]] const std::vector<RHI::ShaderParameter> &GetInputs() const noexcept override { return m_Inputs; }

  [[nodiscard]] bool IsValid() const noexcept { return m_RendererID != 0; }
  [[nodiscard]] u32 GetRendererID() const noexcept { return m_RendererID; }
  [[nodiscard]] bool HasUniform(std::string_view name) { return GetUniformLoc(name) != -1; }

  void Bind() const noexcept { glUseProgram(m_RendererID); }

  void Set(std::string_view name, const RHI::ShaderParamValue &value) {
    std::visit([this, name](const auto &value) { Set(name, value); }, value);
  }

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

  void ResetTextureSlots(u32 first = 0) noexcept { m_NextTextureSlot = first; }
  [[nodiscard]] u32 GetNextTextureSlot() const noexcept { return m_NextTextureSlot; }

private:
  u32 m_RendererID{};
  std::string m_Name;

  std::vector<RHI::ShaderParameter> m_Inputs;
  struct StringHash {
    using is_transparent = void;
    std::size_t operator()(std::string_view value) const noexcept { return std::hash<std::string_view>{}(value); }
  };

  std::unordered_map<std::string, i32, StringHash, std::equal_to<>> m_UniformLocCache;
  u32 m_NextTextureSlot{};
  i32 m_MaxTextureSlots{};

  bool Compile(RHI::ShaderType type, std::string_view source);
  void Reflect();

  [[nodiscard]] i32 GetUniformLoc(std::string_view name);
  [[nodiscard]] u32 AllocateTextureSlot() noexcept { return m_NextTextureSlot++; }
};

} // namespace Jupiter::Rendering::OpenGL
