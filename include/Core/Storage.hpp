#pragma once

#include <utility>
#include <memory>
#include <cstring>

namespace Jupiter::Core {
class Storage {
public:
  Storage() noexcept = default;
  explicit Storage(std::size_t bytes)
      : m_Data(std::make_unique<std::byte[]>(bytes)), m_Size(bytes) {}
  ~Storage() = default;

  Storage(const Storage &other) : Storage(other.m_Size) {
    if (m_Size)
      std::memcpy(m_Data.get(), other.m_Data.get(), m_Size);
  }

  Storage &operator=(const Storage &other) {
    if (this == &other)
      return *this;

    Storage tmp(other);
    std::swap(m_Data, tmp.m_Data);
    std::swap(m_Size, tmp.m_Size);
    return *this;
  }

  Storage(Storage &&other) noexcept = default;
  Storage &operator=(Storage &&other) noexcept = default;

  [[nodiscard]]
  std::byte *data() noexcept {
    return m_Data.get();
  }

  [[nodiscard]]
  const std::byte *data() const noexcept {
    return m_Data.get();
  }

  [[nodiscard]]
  std::size_t size() const noexcept {
    return m_Size;
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return m_Size == 0;
  }

  void resize(std::size_t size) {
    if (size == m_Size)
      return;

    if (size == 0) {
      m_Data.reset();
      m_Size = 0;
      return;
    }

    auto temp = std::make_unique<std::byte[]>(size);
    std::memcpy(temp.get(), data(), size < m_Size ? size : m_Size);

    m_Data = std::move(temp);
    m_Size = size;
  }

  void reserve(std::size_t bytes) {
    if (m_Size < bytes)
      resize(bytes);
  }

  void clear() noexcept {
    m_Data.reset();
    m_Size = 0;
  }

private:
  std::unique_ptr<std::byte[]> m_Data;
  std::size_t m_Size;
};
} // namespace Jupiter::Core
