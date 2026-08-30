#pragma once

#include "Assets/Pool.hpp"

namespace Jupiter::Assets {

template <class T> AssetHandle<T> AssetPool<T>::Reserve() {
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

template <class T> AssetHandle<T> AssetPool<T>::Insert(Ref<T> value) {
  if (!value)
    return {};

  AssetHandle<T> handle = Reserve();
  m_Slots[handle.GetID()].data = std::move(value);
  return handle;
}

template <class T>
bool AssetPool<T>::Replace(AssetHandle<T> handle, Ref<T> value) noexcept {
  if (!value || !IsValid(handle))
    return false;

  m_Slots[handle.GetID()].data = std::move(value);
  return true;
}

template <class T> bool AssetPool<T>::Release(AssetHandle<T> handle) {
  if (!IsValid(handle))
    return false;

  auto &slot = m_Slots[handle.GetID()];
  slot.data.reset();
  slot.alive = false;
  ++slot.gen;

  m_FreeIndices.push_back(handle.GetID());
  return true;
}

template <class T>
Ref<T> AssetPool<T>::Resolve(AssetHandle<T> handle) noexcept {
  if (!IsValid(handle))
    return {};

  return m_Slots[handle.GetID()].data;
}

template <class T>
bool AssetPool<T>::IsValid(AssetHandle<T> handle) const noexcept {
  return handle.IsValid() && handle.GetID() < m_Slots.size() &&
         m_Slots[handle.GetID()].alive &&
         m_Slots[handle.GetID()].gen == handle.GetGen();
}

template class AssetPool<Rendering::Texture>;
template class AssetPool<Rendering::Shader>;
template class AssetPool<Rendering::Mesh>;
template class AssetPool<Rendering::Font>;
template class AssetPool<Rendering::Material>;

} // namespace Jupiter::Assets
