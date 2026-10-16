#pragma once

#include "Assets/Pool.hpp"

#include "Assets/AssetTraits.hpp"

#include "Rendering/Texture2D.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Font.hpp"
#include "Rendering/Sampler.hpp"
#include "Rendering/Material.hpp"

namespace Jupiter::Assets {
class AssetPools {
public:
  template <Asset T> AssetPool<T> &GetPool() noexcept {
    constexpr auto type = AssetTraits<T>::type;

    if constexpr (type == AssetType::Texture)
      return m_TexturePool;
    else if constexpr (type == AssetType::Shader)
      return m_ShaderPool;
    else if constexpr (type == AssetType::Mesh)
      return m_MeshPool;
    else if constexpr (type == AssetType::Font)
      return m_FontPool;
    else if constexpr (type == AssetType::Material)
      return m_MaterialPool;
    else if constexpr (type == AssetType::Sampler)
      return m_SamplerPool;
    else
      static_assert(dependent_false<T>, "No AssetPool registered for this asset type");
  }

  template <Asset T> const AssetPool<T> &GetPool() const noexcept {
    constexpr auto type = AssetTraits<T>::type;

    if constexpr (type == AssetType::Texture)
      return m_TexturePool;
    else if constexpr (type == AssetType::Shader)
      return m_ShaderPool;
    else if constexpr (type == AssetType::Mesh)
      return m_MeshPool;
    else if constexpr (type == AssetType::Font)
      return m_FontPool;
    else if constexpr (type == AssetType::Material)
      return m_MaterialPool;
    else if constexpr (type == AssetType::Sampler)
      return m_SamplerPool;
    else
      static_assert(dependent_false<T>, "No AssetPool registered for this asset type");
  }

  // WARNING:
  // Must be called on the render thread because Ref<T> destructors may
  // destroys GPU resources
  void Clear() noexcept {
    m_TexturePool.Clear();
    m_ShaderPool.Clear();
    m_MeshPool.Clear();
    m_FontPool.Clear();
    m_MaterialPool.Clear();
    m_SamplerPool.Clear();
  }

private:
  AssetPool<Rendering::Texture2D> m_TexturePool;
  AssetPool<Rendering::Shader> m_ShaderPool;
  AssetPool<Rendering::Mesh> m_MeshPool;
  AssetPool<Rendering::Font> m_FontPool;
  AssetPool<Rendering::Material> m_MaterialPool;
  AssetPool<Rendering::Sampler> m_SamplerPool;
};

} // namespace Jupiter::Assets
