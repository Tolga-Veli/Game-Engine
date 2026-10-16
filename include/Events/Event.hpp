#pragma once

#include "Core/Utils.hpp"

#include <string_view>

namespace Jupiter::Core {
enum class EventType : u8 {
  None,

  WindowClose,
  WindowResize,
  WindowFramebufferResize,
  WindowFocus,
  WindowLostFocus,
  WindowMoved,
  WindowRefresh,

  KeyPressed,
  KeyReleased,

  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled,
};

class Event {
public:
  virtual ~Event() noexcept = default;

  virtual EventType GetType() const noexcept = 0;
  virtual std::string_view GetName() const noexcept = 0;

  bool IsHandled() const noexcept { return m_Handled; }
  void SetHandled(bool handled) noexcept { m_Handled = handled; }

private:
  bool m_Handled{false};
};

#define EVENT_CLASS_TYPE(type)                                                                                         \
  static EventType GetStaticType() noexcept { return EventType::type; }                                                \
  EventType GetType() const noexcept override { return GetStaticType(); }                                              \
  std::string_view GetName() const noexcept override { return #type; }

} // namespace Jupiter::Core
