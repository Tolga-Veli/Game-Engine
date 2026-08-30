#pragma once

#include "AssetPools.hpp"
#include "AssetTraits.hpp"

#include "Traits/TextureTraits.hpp"
#include "Traits/ShaderTraits.hpp"
#include "Traits/MeshTraits.hpp"
#include "Traits/MaterialTraits.hpp"
#include "Traits/FontTraits.hpp"

namespace Jupiter::Assets {
template <typename T> AssetPool<T> &AssetPools::GetPool() noexcept {
  constexpr auto type = AssetTraits<T>::Type;

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
  else
    static_assert(dependent_false<T>,
                  "No AssetPool registered for this asset type");
}

template <typename T> const AssetPool<T> &AssetPools::GetPool() const noexcept {
  constexpr auto type = AssetTraits<T>::Type;

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
  else
    static_assert(dependent_false<T>,
                  "No AssetPool registered for this asset type");
}

} // namespace Jupiter::Assets
