#pragma once

#include "Rendering/Shader.hpp"

namespace Jupiter::Rendering::RHI {

struct MaterialData {};

using MaterialParams = std::unordered_map<std::string_view, Rendering::RHI::ShaderParamValue>;

class IMaterial {
public:
  virtual ~IMaterial() = default;

  virtual void SetShader(Assets::AssetHandle<Shader> shader) = 0;
  virtual void SetProperty(std::string_view name, const ShaderParamValue &val) = 0;
  virtual void RemoveProperty(std::string_view name) = 0;
  virtual void ClearProperties() = 0;
};
} // namespace Jupiter::Rendering::RHI
