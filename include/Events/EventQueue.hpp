#ifdef HERE
#pragma once

#include "Event.hpp"

namespace Jupiter::Core {
class EventQueue {
private:
  template <class U>
    requires(std::constructible_from<U, Event>)
  void Push(U &&val) {
    m_Queue.push_back(std::forward<U>(val));
  }

public:
  std::vector<Event> m_Queue;
};
} // namespace Jupiter::Core
#endif
