#pragma once

#include "Core/Utils.hpp"
#include "Types.hpp"

namespace Jupiter::Assets {

inline constexpr AssetID k_InvalidAssetIndex =
    std::numeric_limits<AssetID>::max();

template <class T> struct AssetHandle {
  constexpr AssetHandle() noexcept = default;
  constexpr AssetHandle(AssetID id, u64 gen) noexcept
      : m_ID(id), m_Generation(gen) {}

  constexpr AssetHandle(const AssetHandle &) noexcept = default;
  constexpr AssetHandle &operator=(const AssetHandle &) noexcept = default;
  constexpr AssetHandle(AssetHandle &&) noexcept = default;
  constexpr AssetHandle &operator=(AssetHandle &&) noexcept = default;

  [[nodiscard]] constexpr bool IsValid() const noexcept {
    return m_ID != k_InvalidAssetIndex;
  }

  [[nodiscard]]
  constexpr AssetID GetID() const noexcept {
    return m_ID;
  }

  [[nodiscard]]
  constexpr u64 GetGen() const noexcept {
    return m_Generation;
  }

  constexpr bool operator==(const AssetHandle &other) const noexcept = default;

private:
  AssetID m_ID{k_InvalidAssetIndex};
  u64 m_Generation{};
};

} // namespace Jupiter::Assets
