#pragma once

#include <cstddef>
#include <array>
#include <atomic>
#include <type_traits>
#include <concepts>
#include <utility>
#include <memory>

namespace Jupiter::Data {
inline consteval bool IsPowOf2(std::size_t sz) { return sz != 0 && (sz & (sz - 1)) == 0; }

template <class T, std::size_t N>
  requires(N > 1 && IsPowOf2(N))
class SPSC_Queue {
public:
  SPSC_Queue() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
  ~SPSC_Queue() = default;

  SPSC_Queue(const SPSC_Queue &) = delete;
  SPSC_Queue &operator=(const SPSC_Queue &) = delete;
  SPSC_Queue(SPSC_Queue &&) = delete;
  SPSC_Queue &operator=(SPSC_Queue &&) = delete;

  template <class U>
    requires(std::is_assignable_v<T &, U &&>)
  bool try_push(U &&val) noexcept(std::is_nothrow_assignable_v<T &, U &&>) {
    const std::size_t head = m_Head.load(std::memory_order_relaxed), next = (head + 1) & k_Mask;
    if (next == m_TailCache) {
      m_TailCache = m_Tail.load(std::memory_order_acquire);
      if (next == m_TailCache)
        return false;
    }

    m_Buffer[head] = std::forward<U>(val);
    m_Head.store(next, std::memory_order_release);
    return true;
  }

  bool try_pop(T &val) noexcept(std::is_nothrow_move_assignable_v<T>) {
    const std::size_t tail = m_Tail.load(std::memory_order_relaxed);
    if (tail == m_HeadCache) {
      m_HeadCache = m_Head.load(std::memory_order_acquire);
      if (tail == m_HeadCache)
        return false;
    }

    val = std::move(m_Buffer[tail]);
    m_Tail.store((tail + 1) & k_Mask, std::memory_order_release);
    return true;
  }

  [[nodiscard]] bool empty() const noexcept {
    return m_Head.load(std::memory_order_acquire) == m_Tail.load(std::memory_order_acquire);
  }

  [[nodiscard]] bool full() const noexcept {
    const std::size_t head = (m_Head.load(std::memory_order_acquire) + 1) & k_Mask,
                      tail = m_Tail.load(std::memory_order_acquire);
    return head == tail;
  }

private:
  static constexpr std::size_t k_Mask = N - 1;
  std::array<T, N> m_Buffer{};

  // Producer:
  alignas(64) std::atomic<std::size_t> m_Head{};
  alignas(64) std::size_t m_TailCache{};

  // Consumer:
  alignas(64) std::atomic<std::size_t> m_Tail{};
  alignas(64) std::size_t m_HeadCache{};
};
} // namespace Jupiter::Data
