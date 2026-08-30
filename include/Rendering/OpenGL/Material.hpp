#pragma once

#include "Rendering/Backend/IMaterial.hpp"
#include "Rendering/Backend/IShader.hpp"

#include <unordered_map>
#include <string>

namespace Jupiter::Rendering::OpenGL {
class Material final : public RHI::IMaterial {
public:
  Material() = default;
  ~Material() override = default;

  void SetShader(Assets::AssetHandle<Shader> shader) override;

  void SetProperty(std::string_view name,
                   const RHI::ShaderParamValue &val) override;

  void RemoveProperty(std::string_view name) override;

  void ClearProperties() override;

  void Bind();

private:
  Assets::AssetHandle<Shader> m_ShaderHandle{};
  std::unordered_map<std::string, RHI::ShaderParamValue> m_Properties;
};

} // namespace Jupiter::Rendering::OpenGL
