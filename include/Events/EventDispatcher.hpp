#pragma once

#include "Event.hpp"

namespace Jupiter::Core {
// Routes a single Event to a handler. Construct one per incoming event, then
// Dispatch<T> against it for each concrete type you care about - only the call
// whose T matches the event's actual runtime type ever invokes the handler
//
//  void Application::OnEvent(Event &e){
//    EventDispatcher dispatcher(e);
//    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent&e)->bool{
//      m_Running = false;
//      return true; // true = "handled"; stored back on the event
//    });
//  }

namespace detail {
template <class T>
concept EventClass =
    std::derived_from<std::remove_cvref_t<T>, Event> && requires {
      { std::remove_cvref_t<T>::GetStaticType() } -> std::same_as<EventType>;
    };

template <class Fn, class T>
concept EventHandler = EventClass<T> && requires(Fn &&handler, T &event) {
  { std::invoke(std::forward<Fn>(handler), event) } -> std::same_as<bool>;
};

} // namespace detail

class EventDispatcher {
public:
  explicit EventDispatcher(Event &event) : m_Event(event) {}

  template <detail::EventClass T, detail::EventHandler<T> Fn>
  bool Dispatch(Fn &&handler) {
    if (m_Event.GetType() != T::GetStaticType())
      return false;

    auto &event = static_cast<T &>(m_Event);
    const bool fl = std::invoke(std::forward<Fn>(handler), event);

    if (fl)
      m_Event.SetHandled(true);

    return true;
  }

private:
  Event &m_Event;
};
} // namespace Jupiter::Core
