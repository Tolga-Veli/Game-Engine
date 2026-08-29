#pragma once

#include "Core/Utils.hpp"

#include <atomic>
#include <thread>
#include <type_traits>

namespace Jupiter::Data {
template <class T, std::size_t N>
  requires(N > 0 && (N & (N - 1)) == 0) // must be a power of two
class SPSC_Queue {
public:
  SPSC_Queue() = default;
  ~SPSC_Queue() = default;

  SPSC_Queue(const SPSC_Queue &) = delete;
  SPSC_Queue &operator=(const SPSC_Queue &) = delete;
  SPSC_Queue(SPSC_Queue &&) = delete;
  SPSC_Queue &operator=(SPSC_Queue &&) = delete;

  template <class U>
    requires(std::constructible_from<T, U>)
  void push(U &&val) {
    std::size_t tail = m_Tail.load(std::memory_order_relaxed);
    std::size_t next_tail = (tail + 1) & (N - 1);

    while (next_tail == m_Head.load(std::memory_order_acquire))
      std::this_thread::yield(); // queue full

    m_Queue[tail] = std::forward<U>(val);
    m_Tail.store(next_tail, std::memory_order_release);
  }

  void pop(T &out) {}

private:
  std::array<T, N> m_Queue;
  alignas(64) std::atomic<std::size_t> m_Head{}, m_Tail{};
};

} // namespace Jupiter::Data
