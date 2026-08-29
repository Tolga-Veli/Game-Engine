#pragma once

#include "Event.hpp"

namespace Jupiter::Core {
// Routes a single Event to a handler. Construct one per incoming event, then
// Dispatch<T> against it for each concrete type you care about - only the call
// whose T matches the event's actual runtime type ever invokes the handler
//
//  void Application::OnEvent(Event &e){
//    EventDispatcher dispatcher(e);
//    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent&e){
//      m_Running = false;
//      return true; // true = "handled"; stored back on the event
//    });
//  }

class EventDispatcher {
public:
  explicit EventDispatcher(Event &event) : m_Event(event) {}

  template <class T, class Fn> bool Dispatch(const Fn &handler) {
    if (m_Event.GetType() != T::GetStaticType())
      return false;

    m_Event.SetHandled(handler(static_cast<T &>(m_Event)));
    return true;
  }

private:
  Event &m_Event;
};

// wraps a member function into (Event&) -> bool that Dispatch() expects
// Only valid inside a method that has 'this' (e.g. OnEvent)
#define J_BIND_EVENT_FN(fn) [this](auto &event) { return fn(event); }

} // namespace Jupiter::Core
