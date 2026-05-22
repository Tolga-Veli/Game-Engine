#include <vector>
#include <array>
#include <string_view>

#include "Shader.hpp"
#include "Core/Utils.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace Jupiter::Renderer {

void GLShader::SetInt(std::string_view name, i32 value) {
  glUniform1i(GetUniformLocation(name), value);
}
void GLShader::SetFloat(std::string_view name, float value) {
  glUniform1f(GetUniformLocation(name), value);
}

void GLShader::SetVec3(std::string_view name, const glm::vec3 &value) {
  glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void GLShader::SetVec4(std::string_view name, const glm::vec4 &value) {
  glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void GLShader::SetMat4(std::string_view name, const glm::mat4 &value) {
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE,
                     glm::value_ptr(value));
}

i32 GLShader::GetUniformLocation(std::string_view name) const {
  const std::string name_str(name);
  if (m_UniformLocationCache.find(name_str) != m_UniformLocationCache.end())
    return m_UniformLocationCache[name_str];

  i32 location = glGetUniformLocation(m_ShaderProgram, name_str.c_str());
  if (location == -1) {
    J_WARN("Uniform '{}' not found in shader!", name);
    return 0;
  }

  m_UniformLocationCache[name_str] = location;
  return location;
}

u32 GLShader::CreateProgram(
    const std::array<std::string, EnumToSize(ShaderType::Count)> &stages) {
  u32 program = glCreateProgram();
  std::vector<u32> shaderIDs;
  auto sz = EnumToSize(ShaderType::Count);
  for (u32 i = 1; i < sz; i++) {
    if (stages[i].empty())
      continue;

    u32 id = CompileShader(ShaderType{i}, stages[i]);
    if (id != 0) {
      glAttachShader(program, id);
      shaderIDs.push_back(id);
    }
  }

  glLinkProgram(program);

  i32 status = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (!status) {
    i32 length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

    std::vector<char> info(length);
    glGetProgramInfoLog(program, length, &length, info.data());
    glDeleteProgram(program);

    J_ERROR("Shader linking Failed: {}", info.data());
    return 0;
  }

  for (auto id : shaderIDs) {
    glDetachShader(program, id);
    glDeleteShader(id);
  }

  return program;
}

u32 GLShader::CompileShader(ShaderType type, std::string_view source) {
  u32 id = glCreateShader(JupiterToOpenGLShaderStage(ShaderType(type)));

  const char *src = source.data();
  GLint length = source.size();

  glShaderSource(id, 1, &src, &length);
  glCompileShader(id);

  i32 success = 0;
  glGetShaderiv(id, GL_COMPILE_STATUS, &success);
  if (!success) {
    i32 length = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

    std::vector<char> info(length);
    glGetShaderInfoLog(id, length, &length, info.data());

    J_ERROR("{} Shader compilation failed:\n{}", to_string(type), info.data());
    glDeleteShader(id);
    return 0;
  }

  return id;
}

} // namespace Jupiter::Renderer
