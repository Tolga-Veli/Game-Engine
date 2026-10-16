#pragma once

#include "Rendering/Backend/IMaterial.hpp"

namespace Jupiter::Rendering::OpenGL {
class Material final : public RHI::IMaterial {
public:
  Material() = default;
  ~Material() override = default;

  void SetShader(Assets::AssetHandle<Shader> handle) override { m_ShaderHandle = handle; }
  void SetProperty(std::string_view name, const RHI::ShaderParamValue &val) override;
  void RemoveProperty(std::string_view name) override;
  void ClearProperties() override;

  void Bind();

private:
  Assets::AssetHandle<Rendering::Shader> m_ShaderHandle{};
  RHI::MaterialParams m_Properties;
};

} // namespace Jupiter::Rendering::OpenGL
