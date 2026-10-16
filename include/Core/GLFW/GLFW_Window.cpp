#include "Core/GLFW/GLFW_Window.hpp"

#include "Events/KeyboardEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "Events/WindowEvent.hpp"

namespace Jupiter::Core::detail {
namespace {
// GLFW window creation and destruction are main-thread-only.
u32 s_WindowCount{};
} // namespace

GLFW_Window::GLFW_Window(const WindowOpts &opts)
    : m_WindowMode(opts.mode), m_RefreshRate(opts.refresh_rate), m_Title(opts.title) {

  glfwSetErrorCallback(GLFWErrorCallback);

  if (!glfwInit())
    throw std::runtime_error("Window: glfwInit() failed");
  ++s_WindowCount;

  try {
    glfwWindowHint(GLFW_RESIZABLE, opts.resizeable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, opts.decorated ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, opts.floating ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, opts.maximized ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, opts.visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, opts.focused ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, opts.mouse_passthrough ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, opts.samples);
    glfwWindowHint(GLFW_SRGB_CAPABLE, opts.sRGB ? GLFW_TRUE : GLFW_FALSE);

    ApplyClientAPIHints(opts.api);

    m_WindowHandle = glfwCreateWindow(opts.width, opts.height, m_Title.c_str(), nullptr, nullptr);

    if (!m_WindowHandle)
      throw std::runtime_error("Window: glfwCreateWindow() failed");

    if (opts.pos_x != -1 && opts.pos_y != -1)
      glfwSetWindowPos(m_WindowHandle, opts.pos_x, opts.pos_y);

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

    LOG_INFO("Window: created '{}' ({}x{}), platform: {}", m_Title, opts.width, opts.height, glfwGetPlatform());
  } catch (...) {
    if (m_WindowHandle)
      glfwDestroyWindow(m_WindowHandle);
    if (--s_WindowCount == 0)
      glfwTerminate();
    throw;
  }
}

GLFW_Window::~GLFW_Window() {
  if (m_WindowHandle)
    glfwDestroyWindow(m_WindowHandle);
  if (--s_WindowCount == 0)
    glfwTerminate();
}

void GLFW_Window::ApplyClientAPIHints(Rendering::GraphicsAPI api) const {
  switch (api) {
  case Rendering::GraphicsAPI::OpenGL:
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#ifdef J_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    break;
  case Rendering::GraphicsAPI::Vulkan:
  case Rendering::GraphicsAPI::Metal:
  case Rendering::GraphicsAPI::DX11:
  case Rendering::GraphicsAPI::DX12:
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    break;
  default:
    throw std::invalid_argument("Window: requested graphics API is not supported");
  }
}

void GLFW_Window::SetWindowMode(WindowMode mode) noexcept {
  if (mode == m_WindowMode)
    return;

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

void GLFW_Window::ToggleFullscreen() noexcept {
  if (m_WindowMode == WindowMode::Windowed)
    SetWindowMode(WindowMode::Fullscreen);
  else
    SetWindowMode(WindowMode::Windowed);
}

void GLFW_Window::EnterWindowed() {
  // glfwSetWindowMonitor(m_WindowHandle, nullptr, m_WindowState.x, m_WindowState.y, m_WindowState.width,
  //                    m_WindowState.height, GLFW_DONT_CARE);

#if defined(__linux__)
  glfwSetWindowAttrib(m_WindowHandle, GLFW_DECORATED, GLFW_TRUE);
#endif
}

void GLFW_Window::EnterFullscreen() {}

void GLFW_Window::EnterBorderlessFullscreen() {}

void GLFW_Window::FramebufferResizeCallback(GLFWwindow *window, i32 width, i32 height) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));

  WindowFramebufferResizeEvent event(width, height);
  self->FireEvent(event);
}

void GLFW_Window::WindowSizeCallback(GLFWwindow *window, i32 width, i32 height) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));

  WindowResizeEvent event(width, height);
  self->FireEvent(event);
}

void GLFW_Window::WindowPosCallback(GLFWwindow *window, i32 x, i32 y) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));

  WindowMovedEvent event(x, y);
  self->FireEvent(event);
}

void GLFW_Window::WindowCloseCallback(GLFWwindow *window) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));

  WindowCloseEvent event;
  self->FireEvent(event);
}

void GLFW_Window::WindowFocusCallback(GLFWwindow *window, i32 focused) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));

  if (focused == true) {
    WindowFocusEvent event;
    self->FireEvent(event);
  } else {
    WindowLostFocusEvent event;
    self->FireEvent(event);
  }
}

void GLFW_Window::WindowRefreshCallback(GLFWwindow *window) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));

  WindowRefreshEvent event;
  self->FireEvent(event);
}

void GLFW_Window::KeyCallback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));
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

void GLFW_Window::CursorPosCallback(GLFWwindow *window, f64 x, f64 y) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));

  MouseMovedEvent event(x, y);
  self->FireEvent(event);
}

void GLFW_Window::MouseButtonCallback(GLFWwindow *window, i32 button, i32 action, i32 mods) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));
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

void GLFW_Window::ScrollCallback(GLFWwindow *window, f64 offsetX, f64 offsetY) {
  auto *self = static_cast<GLFW_Window *>(glfwGetWindowUserPointer(window));

  MouseScrolledEvent event(offsetX, offsetY);
  self->FireEvent(event);
}
} // namespace Jupiter::Core::detail
