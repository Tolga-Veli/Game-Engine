#pragma once

#include <memory>
#include <cstring>

namespace Jupiter::Core {
// Typed-erased raw memory storage
class Storage {
public:
  Storage() noexcept = default;
  explicit Storage(std::size_t bytes)
      : m_Data(std::make_unique<std::byte[]>(bytes)), m_Size(bytes) {}
  ~Storage() noexcept = default;

  Storage(const Storage &) = delete;
  Storage &operator=(const Storage &) = delete;
  Storage(Storage &&) noexcept = default;
  Storage &operator=(Storage &&) noexcept = default;

  [[nodiscard]] std::byte *data() { return m_Data.get(); }
  [[nodiscard]] const std::byte *data() const { return m_Data.get(); }

  [[nodiscard]]
  std::size_t size() const noexcept {
    return m_Size;
  }

  [[nodiscard]] bool empty() const noexcept { return m_Size == 0; }

  void reset() noexcept {
    m_Data.reset();
    m_Size = 0;
  }

  void resize(std::size_t bytes) {
    if (bytes < m_Size)
      return;

    auto data = std::make_unique<std::byte[]>(bytes);
    if (m_Data)
      std::memcpy(data.get(), m_Data.get(), std::min(m_Size, bytes));

    m_Data = std::move(data);
    m_Size = bytes;
  }

  template <class T> T *as() noexcept {
    return reinterpret_cast<T *>(m_Data.get());
  }

  template <class T> const T *as() const noexcept {
    return reinterpret_cast<const T *>(m_Data.get());
  }

private:
  std::unique_ptr<std::byte[]> m_Data;
  std::size_t m_Size{};
};
} // namespace Jupiter::Core
