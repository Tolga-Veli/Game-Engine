#pragma once

#include "Pool.hpp"

namespace Jupiter::Assets {
class AssetPools {
public:
  template <typename T> AssetPool<T> &GetPool() noexcept;
  template <typename T> const AssetPool<T> &GetPool() const noexcept;

  // WARNING:
  // Must be called on the render thread because Ref<T> destructors may
  // destroys GPU resources
  void Clear() noexcept {
    m_TexturePool.Clear();
    m_ShaderPool.Clear();
    m_MeshPool.Clear();
    m_FontPool.Clear();
    m_MaterialPool.Clear();
  }

private:
  AssetPool<Rendering::Texture> m_TexturePool;
  AssetPool<Rendering::Shader> m_ShaderPool;
  AssetPool<Rendering::Mesh> m_MeshPool;
  AssetPool<Rendering::Font> m_FontPool;
  AssetPool<Rendering::Material> m_MaterialPool;
};

} // namespace Jupiter::Assets
