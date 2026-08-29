#pragma once

#include "Assets/Types.hpp"

#include "Rendering/Backend/IShader.hpp"
#include "Rendering/Backend/ITexture.hpp"

#include <unordered_map>

namespace Jupiter::Rendering {
class Material {
public:
  explicit Material(const Assets::AssetHandle<IShader> &shader)
      : m_ShaderHandle(shader) {}

  virtual ~Material() = default;

  Ref<IShader> Bind() const;
  void Unbind() const;

  Assets::AssetHandle<IShader> GetShaderHandle() const {
    return m_ShaderHandle;
  }

  void SetFloat(std::string name, float value) { m_Floats[name] = value; }

  void SetVec3(std::string name, const glm::vec3 &value) {
    m_Vec3s[name] = value;
  }

  void SetTexture(std::string name, Assets::AssetHandle<ITexture> texture,
                  i32 slot = 0) {
    m_Textures[name] = TextureBinding{texture, slot};
  }

  static Assets::AssetHandle<Material>
  Create(std::string name, const Assets::AssetHandle<IShader> &shader);

private:
  struct TextureBinding {
    Assets::AssetHandle<ITexture> handle;
    i32 slot{};
  };

  Assets::AssetHandle<IShader> m_ShaderHandle;

  std::unordered_map<std::string, float> m_Floats;
  std::unordered_map<std::string, glm::vec3> m_Vec3s;
  std::unordered_map<std::string, TextureBinding> m_Textures;
};

} // namespace Jupiter::Rendering
