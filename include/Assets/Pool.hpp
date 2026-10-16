#pragma once

#include "Core/Utils.hpp"
#include "AssetHandle.hpp"

#include <vector>

namespace Jupiter::Assets {

template <class T> class AssetPool {
public:
  AssetPool() noexcept = default;
  ~AssetPool() noexcept = default;

  AssetHandle<T> Reserve();
  AssetHandle<T> Insert(Ref<T> value);
  Ref<T> Resolve(AssetHandle<T> handle) const noexcept;

  // hot-swaps a slot's payload, keeping the same index and generation. Anyone
  // holding this handle sees the new data the next time they call Resolve()
  bool Replace(AssetHandle<T> handle, Ref<T> value) noexcept;

  // Frees a slot and bumps its generation, so any handle still pointing at it
  // fails validation instead of silently resolving to whatever gets allocated
  // into that slot next
  bool Release(AssetHandle<T> handle);

  bool IsValid(AssetHandle<T> handle) const noexcept;
  void Clear() noexcept;

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

#include "Pool.inl"
