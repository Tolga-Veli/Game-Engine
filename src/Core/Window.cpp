#include "Core/Window.hpp"
#include "Core/Utils.hpp"
#include "Core/Assert.hpp"

#include "Events/KeyboardEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "Events/WindowEvent.hpp"

namespace Jupiter::Core {

u32 Window::s_GLFWwindowCount = 0;

static void GLFWErrorCallback(i32 error, const char *desc) {
  LOG_ERROR("GLFW error ({}): {}", error, desc);
}

void Window::ApplyClientAPIHints(Rendering::GraphicsAPI api) {
  switch (api) {
  case Rendering::GraphicsAPI::OpenGL:
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    // glfwWindowHint(GLFW_PLATFORM, GLFW_X11_INSTANCE_NAME);
#ifdef J_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    break;
  default:
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    CORE_VERIFY(false, "not yet implemented");
  }
}

Window::Window(const WindowOpts &opts)
    : m_RefreshRate(opts.refresh_rate), m_Title(opts.title),
      m_WindowMode(opts.mode), m_Data{opts.width, opts.height},
      m_WindowedState{0, 0, opts.width, opts.height} {

  // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

  if (s_GLFWwindowCount == 0) {
    i32 success = glfwInit();
    CORE_VERIFY(success, "Window: glfwInit() failed");
    glfwSetErrorCallback(GLFWErrorCallback);
  }

  // GLFW window hints
  glfwWindowHint(GLFW_RESIZABLE, opts.resizeable ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_DECORATED, opts.decorated ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_FLOATING, opts.floating ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_MAXIMIZED, opts.maximized ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, opts.visible ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_FOCUSED, opts.focused ? GLFW_TRUE : GLFW_FALSE);

#if defined(GLFW_MOUSE_PASSTHROUGH)
  // Requires GLFW 3.4+; guarded in case an older GLFW is vendored somewhere
  glfwWindowHint(GLFW_MOUSE_PASSTHROUGH,
                 opts.mouse_passthrough ? GLFW_TRUE : GLFW_FALSE);
#endif
  glfwWindowHint(GLFW_SAMPLES, opts.samples);
  glfwWindowHint(GLFW_SRGB_CAPABLE, opts.sRGB ? GLFW_TRUE : GLFW_FALSE);

  ApplyClientAPIHints(opts.api);

  m_Monitor = opts.monitor;

  m_WindowHandle = glfwCreateWindow(opts.width, opts.height, m_Title.c_str(),
                                    nullptr, nullptr);

  CORE_ASSERT(m_WindowHandle, "Window: glfwCreateWindow() failed");
  ++s_GLFWwindowCount;

  if (opts.pos_x != -1 && opts.pos_y != -1) {
    glfwSetWindowPos(m_WindowHandle, opts.pos_x, opts.pos_y);

    m_WindowedState.x = opts.pos_x;
    m_WindowedState.y = opts.pos_y;
  }

  glfwSetWindowUserPointer(m_WindowHandle, this);

  glfwSetWindowPosCallback(m_WindowHandle, WindowPosCallback);
  glfwSetFramebufferSizeCallback(m_WindowHandle, FramebufferResizeCallback);
  glfwSetWindowSizeCallback(m_WindowHandle, WindowSizeCallback);
  glfwSetWindowCloseCallback(m_WindowHandle, WindowCloseCallback);
  glfwSetWindowFocusCallback(m_WindowHandle, WindowFocusCallback);
  glfwSetWindowRefreshCallback(m_WindowHandle, WindowRefreshCallback);

  glfwSetKeyCallback(m_WindowHandle, KeyCallback);
  glfwSetCursorPosCallback(m_WindowHandle, CursorPosCallback);
  glfwSetMouseButtonCallback(m_WindowHandle, MouseButtonCallback);
  glfwSetScrollCallback(m_WindowHandle, ScrollCallback);

  if (opts.mode != WindowMode::Windowed)
    SetWindowMode(opts.mode);

  LOG_INFO("Window: created '{}' ({}x{}), platform: {}", m_Title, opts.width,
           opts.height, glfwGetPlatform());
}

Window::~Window() {
  if (m_WindowHandle)
    glfwDestroyWindow(m_WindowHandle);

  if (--s_GLFWwindowCount == 0)
    glfwTerminate();
}

GLFWmonitor *Window::GetTargetMonitor() const {
  if (m_Monitor)
    return m_Monitor;

  GLFWmonitor *target = glfwGetPrimaryMonitor();
  CORE_ASSERT(target, "Window: no monitor available");

  return target;
}

void Window::SetWindowMode(WindowMode mode) {
  if (mode == m_WindowMode)
    return;

  if (m_WindowMode == WindowMode::Windowed) {
    m_WindowedState.width = m_Data.width;
    m_WindowedState.height = m_Data.height;
  }

  using enum WindowMode;
  switch (mode) {
  case Windowed:
    EnterWindowed();
    break;

  case Fullscreen:
    EnterFullscreen();
    break;

  case BorderlessFullscreen:
    EnterBorderlessFullscreen();
    break;
  }

  m_WindowMode = mode;
}

void Window::ToggleFullscreen() {
  if (m_WindowMode == WindowMode::Windowed)
    SetWindowMode(WindowMode::Fullscreen);
  else
    SetWindowMode(WindowMode::Windowed);
}

void Window::EnterWindowed() {
  glfwSetWindowMonitor(m_WindowHandle, nullptr, m_WindowedState.x,
                       m_WindowedState.y, m_WindowedState.width,
                       m_WindowedState.height, GLFW_DONT_CARE);

#if defined(__linux__)
  glfwSetWindowAttrib(m_WindowHandle, GLFW_DECORATED, GLFW_TRUE);
#endif
}

void Window::EnterFullscreen() {
  GLFWmonitor *target = GetTargetMonitor();
  CORE_VERIFY(target, "Window: could not find target monitor");

  const GLFWvidmode *mode = glfwGetVideoMode(target);
  CORE_VERIFY(mode, "Window: could not query video mode for target monitor");

  const i32 refreshRate =
      (m_RefreshRate == GLFW_DONT_CARE) ? mode->refreshRate : m_RefreshRate;

#if defined(__linux__)
  glfwSetWindowAttrib(m_WindowHandle, GLFW_DECORATED, GLFW_FALSE);
#endif

  glfwSetWindowMonitor(m_WindowHandle, target, 0, 0, mode->width, mode->height,
                       refreshRate);
}

void Window::EnterBorderlessFullscreen() {
  GLFWmonitor *target = GetTargetMonitor();
  CORE_ASSERT(target, "Window: could not find target monitor");

  int mx, my;
  glfwGetMonitorPos(target, &mx, &my);

  const GLFWvidmode *mode = glfwGetVideoMode(target);
  CORE_ASSERT(mode, "Window: could not query video mode for target monitor");

  // "Windowed fullscreen": undecorated, sized/positioned to exactly cover the
  // monitor at its current desktop resolution. Deliberately NOT an exclusive
  // video-mode switch (that's what EnterFullscreen() does) -- it stays a
  // regular top-level window, which is what makes alt-tabbing out of it cheap.
  glfwSetWindowAttrib(m_WindowHandle, GLFW_DECORATED, GLFW_FALSE);
  glfwSetWindowPos(m_WindowHandle, mx, my);
  glfwSetWindowSize(m_WindowHandle, mode->width, mode->height);
}

void Window::FramebufferResizeCallback(GLFWwindow *window, i32 width,
                                       i32 height) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

  WindowFramebufferResizeEvent event(width, height);
  self->FireEvent(event);
}

void Window::WindowSizeCallback(GLFWwindow *window, i32 width, i32 height) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

  self->m_Data.width = width;
  self->m_Data.height = height;

  if (self->m_WindowMode == WindowMode::Windowed) {
    self->m_WindowedState.width = width;
    self->m_WindowedState.height = height;
  }

  WindowResizeEvent event(width, height);
  self->FireEvent(event);
}

void Window::WindowPosCallback(GLFWwindow *window, i32 x, i32 y) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

  if (self->m_WindowMode == WindowMode::Windowed) {
    self->m_WindowedState.x = x;
    self->m_WindowedState.y = y;
  }

  WindowMovedEvent event(x, y);
  self->FireEvent(event);
}

void Window::WindowCloseCallback(GLFWwindow *window) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

  WindowCloseEvent event;
  self->FireEvent(event);
}

void Window::WindowFocusCallback(GLFWwindow *window, i32 focused) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

  if (focused == true) {
    WindowFocusEvent event;
    self->FireEvent(event);
  } else {
    WindowLostFocusEvent event;
    self->FireEvent(event);
  }
}

void Window::WindowRefreshCallback(GLFWwindow *window) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

  WindowRefreshEvent event;
  self->FireEvent(event);
}

void Window::KeyCallback(GLFWwindow *window, i32 key, i32 scancode, i32 action,
                         i32 mods) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));
  const KeyCode code = GLFWKeyToKeyCode(key);

  switch (action) {
  case GLFW_PRESS: {
    KeyPressedEvent event(code, false, mods);
    self->FireEvent(event);
    break;
  }
  case GLFW_REPEAT: {
    KeyPressedEvent event(code, true, mods);
    self->FireEvent(event);
    break;
  }
  case GLFW_RELEASE: {
    KeyReleasedEvent event(code, mods);
    self->FireEvent(event);
    break;
  }
  default:
    break;
  }

  (void)scancode;
}

void Window::CursorPosCallback(GLFWwindow *window, f64 x, f64 y) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

  MouseMovedEvent event(x, y);
  self->FireEvent(event);
}

void Window::MouseButtonCallback(GLFWwindow *window, i32 button, i32 action,
                                 i32 mods) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));
  const MouseButton mouseButton = GLFWMouseButtonToButton(button);

  if (action == GLFW_PRESS) {
    MouseButtonPressedEvent event(mouseButton, mods);
    self->FireEvent(event);
  } else if (action == GLFW_RELEASE) {
    MouseButtonReleasedEvent event(mouseButton, mods);
    self->FireEvent(event);
  }

  (void)mods;
}

void Window::ScrollCallback(GLFWwindow *window, f64 offsetX, f64 offsetY) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

  MouseScrolledEvent event(offsetX, offsetY);
  self->FireEvent(event);
}
}; // namespace Jupiter::Core
