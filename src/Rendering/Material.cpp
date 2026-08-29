#include "Material.hpp"

#include "Assets/AssetManager.hpp"

namespace Jupiter::Rendering {
Ref<IShader> Material::Bind() const {
  auto shader = Assets::AssetManager::Resolve(m_ShaderHandle);
  if (!shader)
    return {};

  shader->Bind();

  for (const auto &[name, value] : m_Floats)
    shader->SetFloat(name, value);

  for (const auto &[name, value] : m_Vec3s)
    shader->SetVec3(name, value);

  for (const auto &[name, binding] : m_Textures) {
    auto texture = Assets::AssetManager::Resolve(binding.handle);
    if (texture)
      continue;

    texture->Bind(binding.slot);
    shader->SetInt(name, binding.slot);
  }

  return shader;
}

void Material::Unbind() const {
  auto shader = Assets::AssetManager::Resolve(m_ShaderHandle);

  if (shader)
    shader->Unbind();
}

Assets::AssetHandle<Material>
Material::Create(std::string name, const Assets::AssetHandle<IShader> &shader) {
  return Assets::AssetManager::Create<Material>(
      name, [shader]() -> Ref<Material> {
        return std::make_shared<Material>(shader);
      });
}

} // namespace Jupiter::Rendering
