#include "Rendering/OpenGL/Material.hpp"
#include "Rendering/OpenGL/Shader.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Traits/ShaderTraits.hpp"

#include <string>
#include <string_view>

namespace Jupiter::Rendering::OpenGL {

void Material::SetProperty(std::string_view name, const RHI::ShaderParamValue &val) { m_Properties[name] = val; }

void Material::RemoveProperty(std::string_view name) { m_Properties.erase(name); }

void Material::ClearProperties() { m_Properties.clear(); }

void Material::Bind() {
  auto shader = Assets::AssetManager::Resolve<Rendering::Shader>(m_ShaderHandle);
  CORE_ASSERT(shader, "Material: failed to resolve shader");

  auto &impl = static_cast<OpenGL::Shader &>(shader->GetImpl());
  impl.Bind();

  for (const auto &[name, value] : m_Properties) {
    std::visit([&](const auto &value) { impl.Set(name, value); }, value);
  }
}
} // namespace Jupiter::Rendering::OpenGL
