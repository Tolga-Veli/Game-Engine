#include "Shader.hpp"
#include "Core/Utils.hpp"
#include "Core/Logging.hpp"

#include <vector>
#include <array>
#include <string_view>

namespace Jupiter::Rendering::OpenGL {
i32 Shader::GetUniformLocation(std::string_view name) const {
  const std::string name_str(name);
  if (m_UniformLocationCache.find(name_str) != m_UniformLocationCache.end())
    return m_UniformLocationCache[name_str];

  i32 location = glGetUniformLocation(m_ShaderProgram, name_str.c_str());
  if (location == -1) {
    LOG_ERROR("Uniform '{}' not found in shader!", name);
    m_UniformLocationCache.emplace(name_str, -1);
    return -1;
  }

  m_UniformLocationCache[name_str] = location;
  return location;
}

u32 Shader::CreateProgram(
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

    LOG_ERROR("Shader linking Failed: {}", info.data());
    return 0;
  }

  for (auto id : shaderIDs) {
    glDetachShader(program, id);
    glDeleteShader(id);
  }

  return program;
}

u32 Shader::CompileShader(ShaderType type, std::string_view source) {
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

    LOG_ERROR("{} Shader compilation failed:\n{}", to_string(type),
              info.data());
    glDeleteShader(id);
    return 0;
  }

  return id;
}

} // namespace Jupiter::Rendering::OpenGL
