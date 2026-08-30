#include "Material.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/Backend/IShader.hpp"

#include <string>
#include <string_view>

namespace Jupiter::Rendering::OpenGL {
void Material::SetShader(Assets::AssetHandle<Shader> handle) {
  m_ShaderHandle = std::move(handle);
}

void Material::SetProperty(std::string_view name, const ShaderParamValue &val) {
  m_Properties.insert_or_assign(std::string(name), val);
}

void Material::RemoveProperty(std::string_view name) {
  m_Properties.erase(std::string(name));
}

void Material::ClearProperties() { m_Properties.clear(); }

void Material::Bind() {
  auto shader = Assets::AssetManager::Get<Shader>(m_ShaderHandle);
  shader->Bind();

  u32 textureUnit = 0;
  for (const auto &[name, val] : m_Properties) {
    std::visit([&](const auto &value) {}, val);
  }
}
} // namespace Jupiter::Rendering::OpenGL
