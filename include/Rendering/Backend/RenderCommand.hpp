#pragma once

#include "Core/Utils.hpp"
#include "Rendering/Backend/IShader.hpp"
#include "Rendering/Backend/ShaderTypes.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Texture2D.hpp"

#include <variant>
#include <vector>

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
  Ref<Texture2D> texture;

  glm::mat4 transform{1.0f};
  u32 instanceCount{1}, elementCount{};

  std::vector<std::byte> vertexData, instanceData;
  bool alphaBlend{false}, depthTest{true};

  std::unordered_map<std::string, ShaderParamValue> overrides;

  RenderCommand &ApplyTransform(const glm::mat4 &t) {
    transform = t;
    return *this;
  }

  RenderCommand &SetInstances(u32 count) {
    instanceCount = count;
    return *this;
  }
  RenderCommand &SetElementCount(u32 count) {
    elementCount = count;
    return *this;
  }

  RenderCommand &ApplyOverride(std::string name, ShaderParamValue value) {
    overrides.emplace(std::move(name), std::move(value));
    return *this;
  }

  template <typename T> RenderCommand &UploadVertices(std::span<const T> data) {
    const auto bytes = std::as_bytes(data);
    vertexData.assign(bytes.begin(), bytes.end());
    return *this;
  }

  template <typename T>
  RenderCommand &UploadInstances(std::span<const T> data) {
    const auto bytes = std::as_bytes(data);
    instanceData.assign(bytes.begin(), bytes.end());
    return *this;
  }

  RenderCommand &EnableAlphaBlend() {
    alphaBlend = true;
    return *this;
  }

  RenderCommand &DisableDepthTest() {
    depthTest = false;
    return *this;
  }

  RenderCommand &BindTexture(Ref<Texture2D> value) {
    texture = std::move(value);
    return *this;
  }
};
} // namespace RHI
} // namespace Jupiter::Rendering
