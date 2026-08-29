#pragma once

#include "Event.hpp"

namespace Jupiter::Core {

enum class MouseButton : u8 {
  Unknown = 0,

  Left,
  Right,
  Middle,

  Button4,
  Button5,
  Button6,
  Button7,
  Button8

};

MouseButton GLFWMouseButtonToButton(i32 glfwButton);

class MouseMovedEvent : public Event {
public:
  MouseMovedEvent(f64 x, f64 y) noexcept : m_X(x), m_Y(y) {}

  f64 GetX() const noexcept { return m_X; }
  f64 GetY() const noexcept { return m_Y; }

  EVENT_CLASS_TYPE(MouseMoved);

private:
  f64 m_X, m_Y;
};

class MouseScrolledEvent : public Event {
public:
  MouseScrolledEvent(f64 offsetX, f64 offsetY) noexcept
      : m_OffsetX(offsetX), m_OffsetY(offsetY) {}

  f64 GetOffsetX() const noexcept { return m_OffsetX; }
  f64 GetOffsetY() const noexcept { return m_OffsetY; }

  EVENT_CLASS_TYPE(MouseScrolled)

private:
  f64 m_OffsetX, m_OffsetY;
};

class MouseButtonPressedEvent : public Event {
public:
  MouseButtonPressedEvent(MouseButton button, i32 mods) noexcept
      : m_Button(button), m_Mods(mods) {}

  MouseButton GetButton() const noexcept { return m_Button; }
  i32 GetMods() const noexcept { return m_Mods; }

  EVENT_CLASS_TYPE(MouseButtonPressed)

private:
  MouseButton m_Button;
  i32 m_Mods;
};

class MouseButtonReleasedEvent : public Event {
public:
  MouseButtonReleasedEvent(MouseButton button, i32 mods) noexcept
      : m_Button(button), m_Mods(mods) {}

  MouseButton GetButton() const noexcept { return m_Button; }
  i32 GetMods() const noexcept { return m_Mods; }

  EVENT_CLASS_TYPE(MouseButtonReleased)

private:
  MouseButton m_Button;
  i32 m_Mods;
};

} // namespace Jupiter::Core
