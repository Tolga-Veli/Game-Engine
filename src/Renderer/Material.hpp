#pragma once

#include "Renderer/Backend/IShader.hpp"
#include "Renderer/Backend/ITexture.hpp"
#include "Renderer/Backend/RenderUtils.hpp"

#include <map>

namespace Jupiter::Renderer {

class Material {
public:
  Material(const Ref<IShader> &shader) : m_Shader(shader) {}
  virtual ~Material() = default;

  void Bind() const { m_Shader->Bind(); }
  Ref<IShader> GetShader() const { return m_Shader; }

  void SetFloat(const std::string &name, float value) {
    m_Floats[name] = value;
  }

  void SetVec3(const std::string &name, const glm::vec3 &value) {
    m_Vec3s[name] = value;
  }

  void SetTexture(const std::string &name, const Ref<ITexture> &texture,
                  u32 slot = 0) {
    m_Textures[name] = {texture, slot};
  }

  static Ref<Material> Create(const Ref<IShader> &shader) {
    return std::make_shared<Material>(shader);
  }

  void ApplyParameters() {
    for (const auto &[name, textureData] : m_Textures) {
      textureData.first->Bind(textureData.second);
      m_Shader->SetInt(name, textureData.second);
    }

    for (auto &[name, val] : m_Floats)
      m_Shader->SetFloat(name, val);

    for (auto &[name, val] : m_Vec3s)
      m_Shader->SetVec3(name, val);
  }

private:
  Ref<IShader> m_Shader;
  std::map<std::string, float> m_Floats;
  std::map<std::string, glm::vec3> m_Vec3s;
  std::map<std::string, std::pair<Ref<ITexture>, u32>> m_Textures;
};

} // namespace Jupiter::Renderer
