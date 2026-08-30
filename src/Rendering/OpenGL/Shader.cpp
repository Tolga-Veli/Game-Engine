#include "Shader.hpp"

#include "Core/Assert.hpp"
#include "Core/Logging.hpp"

#include "OpenGL-Utils.hpp"

#include <glm/glm.hpp>

#include <fstream>
#include <sstream>
#include <string>

namespace Jupiter::Rendering::OpenGL {

Shader::Shader(
    const std::array<std::string, EnumToSize(ShaderType::Count)> &shaderSrcs) {
  m_RendererID = glCreateProgram();

  if (!m_RendererID) {
    LOG_ERROR("Failed to create OpenGL shader program.");
    return;
  }

  for (u32 i = 0; i < EnumToSize(ShaderType::Count); ++i) {
    const auto type = static_cast<ShaderType>(i);
    const auto &source = shaderSrcs[i];

    if (source.empty())
      continue;

    Compile(type, source);
  }

  glLinkProgram(m_RendererID);

  i32 success{};
  glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);

  if (!success) {
    i32 infoLength{};
    glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &infoLength);

    std::string infoLog(static_cast<std::size_t>(infoLength), '\0');
    glGetProgramInfoLog(m_RendererID, infoLength, nullptr, infoLog.data());

    LOG_ERROR("Failed to link OpenGL shader program:\n{}", infoLog);

    glDeleteProgram(m_RendererID);
    m_RendererID = 0;
    return;
  }

  Reflect();
}

Shader::~Shader() {
  if (m_RendererID)
    glDeleteProgram(m_RendererID);
}

void Shader::Compile(ShaderType type, std::string_view source) {
  const u32 glType = JupiterToOpenGLShaderStage(type),
            shader = glCreateShader(glType);

  if (shader == 0) {
    LOG_ERROR("Failed to create OpenGL {} shader.", to_string(type));
    return;
  }

  const char *sourcePtr = source.data();
  const i32 sourceLength = static_cast<GLint>(source.size());

  glShaderSource(shader, 1, &sourcePtr, &sourceLength);
  glCompileShader(shader);

  i32 success{};
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    i32 infoLength{};
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

    std::string infoLog(static_cast<std::size_t>(infoLength), '\0');
    glGetShaderInfoLog(shader, infoLength, nullptr, infoLog.data());

    LOG_ERROR("Failed to compile OpenGL {} shader:\n{}", to_string(type),
              infoLog);

    glDeleteShader(shader);
    return;
  }

  glAttachShader(m_RendererID, shader);
  glDeleteShader(shader);
}

void Shader::Reflect() {
  m_Inputs.clear();

  i32 uniformCount = 0;
  glGetProgramiv(m_RendererID, GL_ACTIVE_UNIFORMS, &uniformCount);

  m_Inputs.reserve(static_cast<std::size_t>(uniformCount));
  for (i32 i = 0; i < uniformCount; ++i) {
    constexpr std::size_t MaxNameLength = 256;

    char name[MaxNameLength];
    i32 nameLength{}, size{};
    u32 type{};

    glGetActiveUniform(m_RendererID, static_cast<GLuint>(i), MaxNameLength,
                       &nameLength, &size, &type, name);

    const auto shaderType = OpenGLToJupiterShaderDataType(type);

    if (shaderType == ShaderDataType::None)
      continue;

    m_Inputs.push_back(
        {.name = std::string(name, nameLength), .type = shaderType});
  }
}
} // namespace Jupiter::Rendering::OpenGL
