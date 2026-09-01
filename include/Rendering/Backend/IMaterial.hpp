#pragma once

#include "Rendering/Shader.hpp"

namespace Jupiter::Rendering::RHI {

class IMaterial {
public:
  virtual ~IMaterial() = default;

  static Scope<IMaterial> Create();

  virtual void SetShader(Assets::AssetHandle<Shader> shader) = 0;
  virtual void SetProperty(std::string_view name,
                           const ShaderParamValue &val) = 0;
  virtual void RemoveProperty(std::string_view name) = 0;
  virtual void ClearProperties() = 0;
};
} // namespace Jupiter::Rendering::RHI
