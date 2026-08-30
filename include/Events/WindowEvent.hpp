#pragma once

#include "Event.hpp"

namespace Jupiter::Core {

class WindowCloseEvent : public Event {
public:
  EVENT_CLASS_TYPE(WindowClose)
};

// Logical window size, in screen coordinates - what
// glfwSetWindowSizeCallback reports. On most platforms this tracks the
// framebuffer 1:1, but not e.g. on a HiDPI/Retina display; see
// WindowFramebufferResizeEvent for the pixel size you actually want to feed
// glViewport().
class WindowResizeEvent : public Event {
public:
  WindowResizeEvent(i32 width, i32 height) noexcept
      : m_Width(width), m_Height(height) {}

  i32 GetWidth() const noexcept { return m_Width; }
  i32 GetHeight() const noexcept { return m_Height; }

  EVENT_CLASS_TYPE(WindowResize)

private:
  i32 m_Width, m_Height;
};

// Framebuffer size in pixels - fires from glfwSetFramebufferSizeCallback.
class WindowFramebufferResizeEvent : public Event {
public:
  WindowFramebufferResizeEvent(i32 width, i32 height) noexcept
      : m_Width(width), m_Height(height) {}

  i32 GetWidth() const noexcept { return m_Width; }
  i32 GetHeight() const noexcept { return m_Height; }

  EVENT_CLASS_TYPE(WindowFramebufferResize)

private:
  i32 m_Width, m_Height;
};

class WindowFocusEvent : public Event {
public:
  EVENT_CLASS_TYPE(WindowFocus)
};

class WindowLostFocusEvent : public Event {
public:
  EVENT_CLASS_TYPE(WindowLostFocus)
};

class WindowMovedEvent : public Event {
public:
  WindowMovedEvent(i32 x, i32 y) noexcept : m_X(x), m_Y(y) {}

  i32 GetX() const noexcept { return m_X; }
  i32 GetY() const noexcept { return m_Y; }

  EVENT_CLASS_TYPE(WindowMoved)

private:
  i32 m_X, m_Y;
};

class WindowRefreshEvent : public Event {
public:
  EVENT_CLASS_TYPE(WindowRefresh)
};

} // namespace Jupiter::Core
