#pragma once

#include "Pool.hpp"

namespace Jupiter::Assets {

template <class T> AssetHandle<T> AssetPool<T>::Reserve(AssetID key) {
  u32 idx;
  if (!m_FreeIndices.empty()) {
    idx = m_FreeIndices.back();
    m_FreeIndices.pop_back();
  } else {
    idx = m_Slots.size();
    if (m_Slots.size() >= std::numeric_limits<u32>::max())
      throw std::length_error("Asset pool capacity exceeded");

    if (m_FreeIndices.capacity() <= m_Slots.size())
      m_FreeIndices.reserve(std::max(16uz, m_Slots.size() * 2));

    m_Slots.emplace_back();
  }

  auto &slot = m_Slots[idx];
  slot.data.reset();
  slot.alive = true;
  slot.key = key;
  slot.state = AssetState::Pending;
  return AssetHandle<T>{idx, slot.gen};
}

template <class T> AssetHandle<T> AssetPool<T>::Insert(Ref<T> value) {
  if (!value)
    return {};

  AssetHandle<T> handle = Reserve();
  m_Slots[handle.GetID()].data = std::move(value);
  m_Slots[handle.GetID()].state = AssetState::Ready;
  return handle;
}

template <class T> bool AssetPool<T>::Replace(AssetHandle<T> handle, Ref<T> value) noexcept {
  if (!value || !IsValid(handle))
    return false;

  m_Slots[handle.GetID()].data = std::move(value);
  m_Slots[handle.GetID()].state = AssetState::Ready;
  return true;
}

template <class T> bool AssetPool<T>::Release(AssetHandle<T> handle) {
  if (!IsValid(handle))
    return false;

  auto &slot = m_Slots[handle.GetID()];
  slot.data.reset();
  slot.alive = false;
  slot.state = AssetState::Invalid;
  ++slot.gen;

  m_FreeIndices.push_back(handle.GetID());
  return true;
}

template <class T> Ref<T> AssetPool<T>::Resolve(AssetHandle<T> handle) const noexcept {
  if (!IsValid(handle))
    return {};

  return m_Slots[handle.GetID()].data;
}

template <class T> bool AssetPool<T>::IsValid(AssetHandle<T> handle) const noexcept {
  return handle.IsValid() && handle.GetID() < m_Slots.size() && m_Slots[handle.GetID()].alive &&
         m_Slots[handle.GetID()].gen == handle.GetGen();
}

template <class T> AssetState AssetPool<T>::GetState(AssetHandle<T> handle) const noexcept {
  return IsValid(handle) ? m_Slots[handle.GetID()].state : AssetState::Invalid;
}

template <class T> AssetID AssetPool<T>::GetKey(AssetHandle<T> handle) const noexcept {
  return IsValid(handle) ? m_Slots[handle.GetID()].key : 0;
}

template <class T> void AssetPool<T>::MarkFailed(AssetHandle<T> handle) noexcept {
  if (IsValid(handle) && !m_Slots[handle.GetID()].data)
    m_Slots[handle.GetID()].state = AssetState::Failed;
}

template <class T> void AssetPool<T>::Clear() noexcept {
  m_FreeIndices.clear();
  for (u32 index = 0; index < m_Slots.size(); ++index) {
    auto &slot = m_Slots[index];
    slot.data.reset();
    slot.alive = false;
    slot.state = AssetState::Invalid;
    ++slot.gen;
    m_FreeIndices.push_back(index);
  }
}
} // namespace Jupiter::Assets
