#pragma once

#include "Event.hpp"

namespace Jupiter::Core {
enum class KeyCode : u16 {
  Unknown = 0,

  Space,
  Apostrophe,
  Comma,
  Minus,
  Period,
  Slash,

  Num0,
  Num1,
  Num2,
  Num3,
  Num4,
  Num5,
  Num6,
  Num7,
  Num8,
  Num9,

  Semicolon,
  Equal,

  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,

  LeftBracket,  // [
  Backslash,    // '\'
  RightBracket, // ]
  GraveAccent,  // `

  Escape,
  Enter,
  Tab,
  Backspace,
  Insert,
  Delete,

  Right,
  Left,
  Down,
  Up,

  PageUp,
  PageDown,
  Home,
  End,

  CapsLock,
  ScrollLock,
  NumLock,
  PrintScreen,
  Pause,

  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,

  LeftShift,
  LeftControl,
  LeftAlt,
  LeftSuper,

  RightShift,
  RightControl,
  RightAlt,
  RightSuper,

  Menu,
};

// Translates a GLFW key token into Jupiter Keycode
KeyCode GLFWKeyToKeyCode(i32 glfwKey);

class KeyPressedEvent : public Event {
public:
  KeyPressedEvent(KeyCode key, bool repeat, i32 mods) noexcept
      : m_Key(key), m_Repeat(repeat), m_Mods(mods) {}

  KeyCode GetKeyCode() const noexcept { return m_Key; }
  bool IsRepeat() const noexcept { return m_Repeat; }
  i32 GetMods() const noexcept { return m_Mods; }

  EVENT_CLASS_TYPE(KeyPressed)

private:
  KeyCode m_Key;
  bool m_Repeat;
  i32 m_Mods;
};

class KeyReleasedEvent : public Event {
public:
  KeyReleasedEvent(KeyCode key, i32 mods) noexcept : m_Key(key), m_Mods(mods) {}

  KeyCode GetKeyCode() const noexcept { return m_Key; }
  i32 GetMods() const noexcept { return m_Mods; }

  EVENT_CLASS_TYPE(KeyReleased)

private:
  KeyCode m_Key;
  i32 m_Mods;
};
} // namespace Jupiter::Core
