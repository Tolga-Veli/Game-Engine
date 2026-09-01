#pragma once

#include "Core/Utils.hpp"
#include "AssetHandle.hpp"

#include <vector>

namespace Jupiter::Rendering {
class Texture;
class Shader;
class Mesh;
class Font;
class Material;
} // namespace Jupiter::Rendering

namespace Jupiter::Assets {

template <class T> class AssetPool {
public:
  AssetPool() noexcept = default;
  ~AssetPool() noexcept = default;

  AssetHandle<T> Reserve() {
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

  AssetHandle<T> Insert(Ref<T> value) {
    if (!value)
      return {};

    AssetHandle<T> handle = Reserve();
    m_Slots[handle.GetID()].data = std::move(value);
    return handle;
  }

  // hot-swaps a slot's payload, keeping the same index and generation. Anyone
  // holding this handle sees thew new data the next time they call Resolve()
  bool Replace(AssetHandle<T> handle, Ref<T> value) noexcept {
    if (!value || !IsValid(handle))
      return false;

    m_Slots[handle.GetID()].data = std::move(value);
    return true;
  }

  // Frees a slot and bumps its generation, so any handle still pointing at it
  // fails validation instead of silently resolving to whatever gets allocated
  // into that slot next
  bool Release(AssetHandle<T> handle) {
    if (!IsValid(handle))
      return false;

    auto &slot = m_Slots[handle.GetID()];
    slot.data.reset();
    slot.alive = false;
    ++slot.gen;

    m_FreeIndices.push_back(handle.GetID());
    return true;
  }

  Ref<T> Resolve(AssetHandle<T> handle) const noexcept {
    if (!IsValid(handle))
      return {};

    return m_Slots[handle.GetID()].data;
  }

  bool IsValid(AssetHandle<T> handle) const noexcept {
    return handle.IsValid() && handle.GetID() < m_Slots.size() &&
           m_Slots[handle.GetID()].alive &&
           m_Slots[handle.GetID()].gen == handle.GetGen();
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

} // namespace Jupiter::Assets
