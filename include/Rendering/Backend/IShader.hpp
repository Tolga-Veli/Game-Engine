#pragma once

#include "Render-Utils.hpp"

#include <glm/glm.hpp>

namespace Jupiter::Rendering::RHI {

using ShaderSources = std::array<std::string, EnumToSize(ShaderType::Count)>;

class IShader {
public:
  virtual ~IShader() = default;

  [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;
  [[nodiscard]] virtual const std::vector<ShaderParameter> &GetInputs() const noexcept = 0;

  virtual void Set(std::string_view name, bool value) = 0;
  virtual void Set(std::string_view name, i32 value) = 0;
  virtual void Set(std::string_view name, u32 value) = 0;
  virtual void Set(std::string_view name, f32 value) = 0;

  virtual void Set(std::string_view name, const glm::vec2 &value) = 0;
  virtual void Set(std::string_view name, const glm::vec3 &value) = 0;
  virtual void Set(std::string_view name, const glm::vec4 &value) = 0;

  virtual void Set(std::string_view name, const glm::mat3 &value) = 0;
  virtual void Set(std::string_view name, const glm::mat4 &value) = 0;
  virtual void Set(std::string_view name, const TextureBinding &binding) = 0;
};
} // namespace Jupiter::Rendering::RHI
