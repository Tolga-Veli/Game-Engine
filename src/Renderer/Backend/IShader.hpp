#pragma once

#include "Core/Utils.hpp"
#include "Renderer/Backend/RenderUtils.hpp"

#include <string>
#include <glm/glm.hpp>

namespace Jupiter::Renderer {
class IShader {
public:
  virtual ~IShader() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void SetInt(std::string_view name, i32 value) = 0;
  virtual void SetFloat(std::string_view name, float value) = 0;
  virtual void SetVec3(std::string_view name, const glm::vec3 &value) = 0;
  virtual void SetVec4(std::string_view name, const glm::vec4 &value) = 0;
  virtual void SetMat4(std::string_view name, const glm::mat4 &value) = 0;

  static Ref<IShader>
  Create(const std::array<std::string, EnumToSize(Renderer::ShaderType::Count)>
             &shaderSrcs);
};
} // namespace Jupiter::Renderer
