#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Core/Utils.hpp"
#include "OpenGL-Utils.hpp"
#include "Renderer/Backend/IShader.hpp"

namespace Jupiter::Renderer {

class GLShader : public IShader {
public:
  GLShader() = default;
  GLShader(
      const std::array<std::string, EnumToSize(ShaderType::Count)> &sources) {
    m_ShaderProgram = CreateProgram(sources);
  }

  virtual ~GLShader() { glDeleteProgram(m_ShaderProgram); }

  u32 GetID() const { return m_ShaderProgram; }

  void Bind() const override { glUseProgram(m_ShaderProgram); }
  void Unbind() const override { glUseProgram(0); }

  void SetInt(std::string_view name, i32 value) override;
  void SetFloat(std::string_view name, float value) override;
  void SetVec3(std::string_view name, const glm::vec3 &value) override;
  void SetVec4(std::string_view name, const glm::vec4 &value) override;
  void SetMat4(std::string_view name, const glm::mat4 &value) override;

private:
  u32 m_ShaderProgram{};

  mutable std::unordered_map<std::string, i32> m_UniformLocationCache;

  i32 GetUniformLocation(std::string_view name) const;

  u32 CreateProgram(
      const std::array<std::string, EnumToSize(ShaderType::Count)> &sources);
  u32 CompileShader(ShaderType type, std::string_view source);
};
} // namespace Jupiter::Renderer
