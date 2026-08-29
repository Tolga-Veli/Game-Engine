#pragma once

#include "Core/Utils.hpp"
#include "Core/Logging.hpp"
#include "OpenGL-Utils.hpp"

#include "Rendering/Backend/IShader.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

namespace Jupiter::Rendering::OpenGL {

class Shader final : public IShader {
public:
  Shader() = default;
  Shader(
      const std::array<std::string, EnumToSize(ShaderType::Count)> &sources) {
    m_ShaderProgram = CreateProgram(sources);
  }

  virtual ~Shader() { glDeleteProgram(m_ShaderProgram); }

  u32 GetID() const noexcept { return m_ShaderProgram; }

  void Bind() const override { glUseProgram(m_ShaderProgram); }
  void Unbind() const override { glUseProgram(0); }

  void SetInt(std::string_view name, i32 value) override {
    const i32 loc = GetUniformLocation(name);
    if (loc == -1) {
      LOG_ERROR("Invalid uniform location '{}'", name);
      return;
    }
    glUniform1i(loc, value);
  }

  void SetFloat(std::string_view name, float value) override {
    const i32 loc = GetUniformLocation(name);
    if (loc == -1) {
      LOG_ERROR("Invalid uniform location '{}'", name);
      return;
    }

    glUniform1f(loc, value);
  }

  void SetVec3(std::string_view name, const glm::vec3 &value) override {
    const i32 loc = GetUniformLocation(name);
    if (loc == -1) {
      LOG_ERROR("Invalid uniform location '{}'", name);
      return;
    }

    glUniform3f(loc, value.x, value.y, value.z);
  }

  void SetVec4(std::string_view name, const glm::vec4 &value) override {
    const i32 loc = GetUniformLocation(name);
    if (loc == -1) {
      LOG_ERROR("Invalid uniform location '{}'", name);
      return;
    }

    glUniform4f(loc, value.x, value.y, value.z, value.w);
  }

  void SetMat4(std::string_view name, const glm::mat4 &value) override {
    const i32 loc = GetUniformLocation(name);
    if (loc == -1) {
      LOG_ERROR("Invalid uniform location '{}", name);
      return;
    }

    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
  }

private:
  u32 m_ShaderProgram{};
  mutable std::unordered_map<std::string, i32> m_UniformLocationCache;

  i32 GetUniformLocation(std::string_view name) const;
  u32 CreateProgram(
      const std::array<std::string, EnumToSize(ShaderType::Count)> &sources);
  u32 CompileShader(ShaderType type, std::string_view source);
};
} // namespace Jupiter::Rendering::OpenGL
