#pragma once

#include "EventQueue.hpp"

#include <utility>

namespace Jupiter::Core {

class EventEmitter {
public:
  explicit EventEmitter(EventQueue &queue) : m_Queue(queue) {}

  template <class T, class... Args> void Emit(Args &&...args) {
    m_Queue.Push(T(std::forward<Args>(args)...));
  }

private:
  EventQueue &m_Queue;
};
} // namespace Jupiter::Core
