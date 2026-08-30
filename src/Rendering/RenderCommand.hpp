#pragma once

#include "Core/Utils.hpp"
#include "Rendering/Backend/IShader.hpp"

#include <type_traits>
#include <variant>

namespace Jupiter::Rendering {
class Mesh;
class Material;

using UniformValue = std::variant<i32, float, glm::vec3, glm::vec4, glm::mat4>;

inline void ApplyUniform(IShader &shader, const std::string &name,
                         const UniformValue &value) {
  std::visit(
      [&](const auto &value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, i32>)
          shader.SetInt(name, value);
        else if constexpr (std::is_same_v<T, float>)
          shader.SetFloat(name, value);
        else if constexpr (std::is_same_v<T, glm::vec3>)
          shader.SetVec3(name, value);
        else if constexpr (std::is_same_v<T, glm::vec4>)
          shader.SetVec4(name, value);
        else if constexpr (std::is_same_v<T, glm::mat4>)
          shader.SetMat4(name, value);
      },
      value);
}

struct RenderCommand {
  RenderCommand(const Ref<Mesh> &_mesh, const Ref<Material> &mat,
                const glm::mat4 &t = glm::mat4(1.0f))
      : mesh(_mesh), material(mat), transform(t) {}

  Ref<Mesh> mesh;
  Ref<Material> material;
  glm::mat4 transform{1.0f};
  u32 instanceCount = 1;

  std::unordered_map<std::string, UniformValue> overrides;

  RenderCommand &ApplyTransform(const glm::mat4 &t) {
    transform = t;
    return *this;
  }

  RenderCommand &SetInstances(u32 count) {
    instanceCount = count;
    return *this;
  }

  RenderCommand &ApplyOverride(std::string name, UniformValue value) {
    overrides.emplace(std::move(name), std::move(value));
    return *this;
  }
};
} // namespace Jupiter::Rendering
