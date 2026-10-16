#pragma once

#include <cstddef>
#include <cstring>
#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace Jupiter {
class Storage {
public:
  Storage() noexcept = default;

  explicit Storage(std::size_t size) : m_Data(std::make_unique<std::byte[]>(size)), m_Size(size), m_Capacity(size) {}

  template <class T, std::size_t N> explicit Storage(std::span<T, N> source) : Storage(source.size_bytes()) {
    if (m_Size)
      std::memcpy(m_Data.get(), source.data(), m_Size);
  }

  template <class T, std::size_t N> explicit Storage(T (&source)[N]) : Storage(std::span<T, N>{source}) {}

  template <class T, class Alloc> Storage(const std::vector<T, Alloc> &source) : Storage(std::span<const T>(source)) {}

  // WARNING: Deep copy
  Storage(const Storage &other) : Storage(other.m_Size) {
    if (m_Size)
      std::memcpy(m_Data.get(), other.m_Data.get(), m_Size);
  }

  // WARNING: Deep copy
  Storage &operator=(const Storage &other) {
    if (this == &other)
      return *this;

    Storage tmp(other);
    m_Data.swap(tmp.m_Data);
    std::swap(m_Size, tmp.m_Size);
    std::swap(m_Capacity, tmp.m_Capacity);
    return *this;
  }

  Storage(Storage &&other) noexcept
      : m_Data(std::move(other.m_Data)), m_Size(std::exchange(other.m_Size, 0)),
        m_Capacity(std::exchange(other.m_Capacity, 0)) {}

  Storage &operator=(Storage &&other) noexcept {
    if (this == &other)
      return *this;

    m_Data = std::move(other.m_Data);
    m_Size = std::exchange(other.m_Size, 0);
    m_Capacity = std::exchange(other.m_Capacity, 0);
    return *this;
  }

  [[nodiscard]] std::span<std::byte> as_span() noexcept { return std::span<std::byte>{m_Data.get(), m_Size}; }

  [[nodiscard]] std::span<const std::byte> as_span() const noexcept {
    return std::span<const std::byte>{m_Data.get(), m_Size};
  }

  [[nodiscard]] std::byte *data() noexcept { return m_Data.get(); }
  [[nodiscard]] const std::byte *data() const noexcept { return m_Data.get(); }

  [[nodiscard]] std::size_t size() const noexcept { return m_Size; }
  [[nodiscard]] std::size_t capacity() const noexcept { return m_Capacity; }
  [[nodiscard]] bool empty() const noexcept { return m_Size == 0; }

  void reserve(std::size_t size) {
    if (size <= m_Capacity)
      return;

    auto temp = std::make_unique<std::byte[]>(size);

    if (m_Size)
      std::memcpy(temp.get(), m_Data.get(), m_Size);

    m_Data = std::move(temp);
    m_Capacity = size;
  }

  void resize(std::size_t size) {
    if (size > m_Capacity)
      reserve(size);

    m_Size = size;
  }

  void clear() noexcept { m_Size = 0; }

  void release() noexcept {
    m_Data.reset();
    m_Size = 0;
    m_Capacity = 0;
  }

  // WARNING: Careful when casting to not break alignment (might cause UB)
  template <class T> T *as() noexcept { return reinterpret_cast<T *>(m_Data.get()); }

  // WARNING: Careful when casting to not break alignment (might cause UB)
  template <class T> const T *as() const noexcept { return reinterpret_cast<const T *>(m_Data.get()); }

private:
  std::unique_ptr<std::byte[]> m_Data;
  std::size_t m_Size{}, m_Capacity{};
};

} // namespace Jupiter
