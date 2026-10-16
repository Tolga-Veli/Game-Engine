#include "Rendering/OpenGL/Shader.hpp"
#include "Rendering/OpenGL/OpenGL-Utils.hpp"

#include "Assets/AssetManager.hpp"

#include "Core/Logging.hpp"
#include "Rendering/OpenGL/Texture2D.hpp"
#include "Rendering/OpenGL/Sampler.hpp"

#include <glm/glm.hpp>

#include <string>
#include <limits>

namespace Jupiter::Rendering::OpenGL {

i32 Shader::GetUniformLoc(std::string_view name) {
  if (!m_RendererID)
    return -1;
  if (const auto it = m_UniformLocCache.find(name); it != m_UniformLocCache.end())
    return it->second;

  const std::string key{name};
  const i32 loc = glGetUniformLocation(m_RendererID, key.c_str());
  m_UniformLocCache.emplace(key, loc);
  return loc;
}

Shader::Shader(const RHI::ShaderSources &shaderSrcs) {
  const bool compute = !shaderSrcs[EnumToSize(RHI::ShaderType::Compute)].empty();
  if (compute) {
    for (u32 i = 0; i < EnumToSize(RHI::ShaderType::Count); ++i)
      if (i != EnumToSize(RHI::ShaderType::Compute) && !shaderSrcs[i].empty())
        return;
  } else if (shaderSrcs[EnumToSize(RHI::ShaderType::Vertex)].empty())
    return;

  m_RendererID = glCreateProgram();

  if (!m_RendererID) {
    LOG_ERROR("Failed to create OpenGL shader program.");
    return;
  }

  for (u32 i = 0; i < EnumToSize(RHI::ShaderType::Count); ++i) {
    const auto type = static_cast<RHI::ShaderType>(i);
    const auto &source = shaderSrcs[i];

    if (source.empty())
      continue;

    if (!Compile(type, source)) {
      glDeleteProgram(m_RendererID);
      m_RendererID = 0;
      return;
    }
  }

  glLinkProgram(m_RendererID);

  i32 success{};
  glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);

  if (!success) {
    i32 infoLength{};
    glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &infoLength);

    std::string infoLog(infoLength, '\0');
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

bool Shader::Compile(RHI::ShaderType type, std::string_view source) {
  const u32 glType = JupiterToOpenGLShaderType(type);
  if (!glType || source.size() > static_cast<std::size_t>(std::numeric_limits<i32>::max()))
    return false;
  const u32 shader = glCreateShader(glType);

  if (shader == 0) {
    LOG_ERROR("Failed to create OpenGL {} shader.", to_string(type));
    return false;
  }

  const char *sourcePtr = source.data();
  const i32 sourceLength = source.size();

  glShaderSource(shader, 1, &sourcePtr, &sourceLength);
  glCompileShader(shader);

  i32 success{};
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    i32 infoLength{};
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

    std::string infoLog(infoLength, '\0');
    glGetShaderInfoLog(shader, infoLength, nullptr, infoLog.data());

    LOG_ERROR("Failed to compile OpenGL {} shader:\n{}", to_string(type), infoLog);

    glDeleteShader(shader);
    return false;
  }

  glAttachShader(m_RendererID, shader);
  glDeleteShader(shader);
  return true;
}

void Shader::Reflect() {
  m_Inputs.clear();
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_MaxTextureSlots);

  i32 uniformCount = 0;
  glGetProgramiv(m_RendererID, GL_ACTIVE_UNIFORMS, &uniformCount);

  m_Inputs.reserve(uniformCount);
  for (i32 i = 0; i < uniformCount; ++i) {
    constexpr std::size_t MaxNameLength = 256;

    char name[MaxNameLength];
    i32 nameLength{}, size{};
    u32 type{};

    glGetActiveUniform(m_RendererID, i, MaxNameLength, &nameLength, &size, &type, name);

    const auto shaderType = OpenGLToJupiterDataType(type);
    if (shaderType == RHI::DataType::None)
      continue;

    m_Inputs.push_back(RHI::ShaderParameter{.name = std::string(name, nameLength), .type = shaderType});
  }
}

void Shader::Set(std::string_view name, bool value) {
  const i32 location = GetUniformLoc(name);
  if (location == -1)
    return;

  glProgramUniform1i(m_RendererID, location, value ? 1 : 0);
}

void Shader::Set(std::string_view name, i32 value) {
  const i32 location = GetUniformLoc(name);
  if (location == -1)
    return;

  glProgramUniform1i(m_RendererID, location, value);
}

void Shader::Set(std::string_view name, u32 value) {
  const i32 location = GetUniformLoc(name);
  if (location == -1)
    return;

  glProgramUniform1ui(m_RendererID, location, value);
}

void Shader::Set(std::string_view name, f32 value) {
  const i32 location = GetUniformLoc(name);
  if (location == -1)
    return;

  glProgramUniform1f(m_RendererID, location, value);
}

void Shader::Set(std::string_view name, const glm::vec2 &value) {
  const i32 location = GetUniformLoc(name);
  if (location == -1)
    return;

  glProgramUniform2fv(m_RendererID, location, 1, &value[0]);
}

void Shader::Set(std::string_view name, const glm::vec3 &value) {
  const i32 location = GetUniformLoc(name);
  if (location == -1)
    return;

  glProgramUniform3fv(m_RendererID, location, 1, &value[0]);
}

void Shader::Set(std::string_view name, const glm::vec4 &value) {
  const i32 location = GetUniformLoc(name);
  if (location == -1)
    return;

  glProgramUniform4fv(m_RendererID, location, 1, &value[0]);
}

void Shader::Set(std::string_view name, const glm::mat3 &value) {
  const i32 location = GetUniformLoc(name);
  if (location == -1)
    return;

  glProgramUniformMatrix3fv(m_RendererID, location, 1, GL_FALSE, &value[0][0]);
}

void Shader::Set(std::string_view name, const glm::mat4 &value) {
  const i32 location = GetUniformLoc(name);
  if (location == -1)
    return;

  glProgramUniformMatrix4fv(m_RendererID, location, 1, GL_FALSE, &value[0][0]);
}

void Shader::Set(std::string_view name, const RHI::TextureBinding &binding) {
  const i32 location = GetUniformLoc(name);
  if (location == -1 || m_NextTextureSlot >= static_cast<u32>(m_MaxTextureSlots))
    return;

  const u32 slot = AllocateTextureSlot();
  auto texture = Assets::AssetManager::Resolve<Rendering::Texture2D>(binding.texture);
  glBindTextureUnit(slot, texture ? static_cast<OpenGL::Texture2D &>(texture->GetImpl()).GetRendererID() : 0);
  auto sampler =
      binding.sampler.IsValid() ? Assets::AssetManager::Resolve<Rendering::Sampler>(binding.sampler) : nullptr;
  glBindSampler(slot, sampler ? static_cast<OpenGL::Sampler &>(sampler->GetImpl()).GetRendererID() : 0);
  glProgramUniform1i(m_RendererID, location, slot);
}
} // namespace Jupiter::Rendering::OpenGL
