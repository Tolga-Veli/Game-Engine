#pragma once

#include "Core/Utils.hpp"
#include "Rendering/Backend/IShader.hpp"
#include "Rendering/Backend/ShaderTypes.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Material.hpp"

#include <variant>

namespace Jupiter::Rendering {
namespace RHI {

inline void ApplyUniform(IShader &shader, const std::string &name,
                         const ShaderParamValue &value) {
  std::visit([&](const auto &value) { shader.Set(name, value); }, value);
}

struct RenderCommand {
  RenderCommand(const Ref<Mesh> &_mesh, const Ref<Material> &_material,
                const glm::mat4 &_transform = glm::mat4(1.0f),
                u32 _instanceCount = 1)
      : mesh(_mesh), material(_material), transform(_transform),
        instanceCount(_instanceCount) {}

  Ref<Mesh> mesh;
  Ref<Material> material;
  glm::mat4 transform{1.0f};
  u32 instanceCount{1};

  std::unordered_map<std::string, ShaderParamValue> overrides;

  RenderCommand &ApplyTransform(const glm::mat4 &t) {
    transform = t;
    return *this;
  }

  RenderCommand &SetInstances(u32 count) {
    instanceCount = count;
    return *this;
  }

  RenderCommand &ApplyOverride(std::string name, ShaderParamValue value) {
    overrides.emplace(std::move(name), std::move(value));
    return *this;
  }
};
} // namespace RHI
} // namespace Jupiter::Rendering
