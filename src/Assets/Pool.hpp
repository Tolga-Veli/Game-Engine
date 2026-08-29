#pragma once

#include "Core/Utils.hpp"
#include "Types.hpp"
#include "Traits.hpp"

#include "Rendering/Backend/ITexture.hpp"
#include "Rendering/Backend/IShader.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Font.hpp"

#include <vector>

namespace Jupiter::Assets {

template <class T> class AssetPool {
public:
  AssetHandle<T> Reserve() noexcept {
    u32 idx;
    if (!m_FreeIndices.empty()) {
      idx = m_FreeIndices.back();
      m_FreeIndices.pop_back();
    } else {
      idx = static_cast<u32>(m_Slots.size());
      m_Slots.emplace_back();
    }

    auto &slot = m_Slots[idx];
    slot.data.reset();
    slot.alive = true;
    return AssetHandle<T>{idx, slot.gen};
  }
  AssetHandle<T> Insert(Ref<T> value) noexcept {
    if (!value)
      return {};

    AssetHandle<T> handle = Reserve();
    m_Slots[handle.idx].data = std::move(value);
    return handle;
  }

  // hot-swaps a slot's payload, keeping the same index and generation. Anyone
  // holding this handle sees thew new data the next time they call Resolve()
  bool Replace(AssetHandle<T> handle, Ref<T> value) noexcept {
    if (!value || !IsValid(handle))
      return false;

    m_Slots[handle.idx].data = std::move(value);
    return true;
  }

  // Frees a slot and bumps its generation, so any handle still pointing at it
  // fails validation instead of silently resolving to whatever gets allocated
  // into that slot next
  bool Release(AssetHandle<T> handle) noexcept {
    if (!IsValid(handle))
      return false;

    auto &slot = m_Slots[handle.idx];
    slot.data.reset();
    slot.alive = false;
    ++slot.gen;

    m_FreeIndices.push_back(handle.idx);
    return true;
  }

  const Ref<T> *Resolve(AssetHandle<T> handle) const noexcept {
    if (!IsValid(handle))
      return {};

    return &m_Slots[handle.idx].data;
  }

  bool IsValid(AssetHandle<T> handle) const noexcept {
    return handle.IsValid() && handle.idx < m_Slots.size() &&
           m_Slots[handle.idx].alive && m_Slots[handle.idx].gen == handle.gen;
  }

  void Clear() noexcept {
    m_Slots.clear();
    m_FreeIndices.clear();
  }

private:
  struct Slot {
    Ref<T> data{};
    u32 gen{};
    bool alive{false};
  };

  std::vector<Slot> m_Slots;
  std::vector<u32> m_FreeIndices;
};

class AssetPools {
public:
  template <typename T> AssetPool<T> &GetPool() noexcept {
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

  template <typename T> const AssetPool<T> &GetPool() const noexcept {
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
  AssetPool<Rendering::ITexture> m_TexturePool;
  AssetPool<Rendering::IShader> m_ShaderPool;
  AssetPool<Rendering::Mesh> m_MeshPool;
  AssetPool<Rendering::Font> m_FontPool;
  AssetPool<Rendering::Material> m_MaterialPool;
};
} // namespace Jupiter::Assets
