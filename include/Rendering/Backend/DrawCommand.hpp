#pragma once

#include "Assets/AssetHandle.hpp"
#include "Core/Storage.hpp"
#include "Core/Utils.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Texture2D.hpp"

#include <algorithm>
#include <concepts>
#include <glm/glm.hpp>
#include <string>
#include <utility>
#include <vector>

namespace Jupiter::Rendering::RHI {

struct ShaderOverride {
  std::string name;
  ShaderParamValue value;
};

struct DrawCommand {
  DrawCommand(Assets::AssetHandle<Mesh> _mesh, Assets::AssetHandle<Material> _material,
              const glm::mat4 &_transform = glm::mat4(1.0f), u32 _instanceCount = 1)
      : mesh(_mesh), material(_material), transform(_transform), instanceCount(_instanceCount) {}

  Assets::AssetHandle<Mesh> mesh;
  Assets::AssetHandle<Material> material;
  std::optional<Assets::AssetHandle<Texture2D>> texture;

  glm::mat4 transform{1.0f};
  u32 instanceCount{1}, elementCount{};

  Storage vertices, instanceData;
  bool alphaBlend{false}, depthTest{true};

  std::vector<RHI::ShaderOverride> overrides;

  bool IsValid() const noexcept {
    return mesh.IsValid() && material.IsValid() && (texture.has_value() ? texture->IsValid() : true);
  }

  DrawCommand &ApplyTransform(const glm::mat4 &t) {
    transform = t;
    return *this;
  }

  DrawCommand &SetInstances(u32 count) {
    instanceCount = count;
    return *this;
  }

  DrawCommand &SetElementCount(u32 count) {
    elementCount = count;
    return *this;
  }

  DrawCommand &ApplyOverride(std::string name, RHI::ShaderParamValue value) {
    auto it = std::find_if(overrides.begin(), overrides.end(), [&](const auto &x) { return x.name == name; });
    if (it != overrides.end())
      it->value = std::move(value);
    else
      overrides.push_back({std::move(name), std::move(value)});
    return *this;
  }

  template <class U>
    requires(std::constructible_from<Storage, U>)
  DrawCommand &UploadVertices(U &&data) {
    vertices = Storage(std::forward<U>(data));
    return *this;
  }

  template <class U>
    requires(std::constructible_from<Storage, U>)
  DrawCommand &UploadInstances(U &&data) {
    instanceData = Storage(std::forward<U>(data));
    return *this;
  }

  DrawCommand &EnableAlphaBlend() {
    alphaBlend = true;
    return *this;
  }

  DrawCommand &DisableDepthTest() {
    depthTest = false;
    return *this;
  }

  DrawCommand &BindTexture(Assets::AssetHandle<Texture2D> h) {
    texture = h;
    return *this;
  }
};

} // namespace Jupiter::Rendering::RHI
