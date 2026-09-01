#pragma once

#include "Core/Utils.hpp"
#include "Events/Event.hpp"
#include "Rendering/Renderer.hpp"

#include <GLFW/glfw3.h>

#include <functional>
#include <string>
#include <memory>
#include <utility>

namespace Jupiter::Core {

enum class WindowMode : u8 { Windowed, Fullscreen, BorderlessFullscreen };

struct WindowOpts {
  std::string title = "Jupiter Engine";
  i32 width = 1280, height = 720;

  // Window position. -1 = let GLFW decide
  i32 pos_x = -1, pos_y = -1;

  // Window behaviour
  WindowMode mode = WindowMode::Windowed;

  bool resizeable = true;
  bool decorated = true;
  bool floating = false;
  bool maximized = false;
  bool visible = true;
  bool focused = true;

  // Allow the window to receive input while being transparent to mouse input
  bool mouse_passthrough = false;

  // Monitor selection for fullscreen / borderless fullscreen
  // nullptr = auto-detect
  GLFWmonitor *monitor = nullptr;

  Rendering::GraphicsAPI api = Rendering::GraphicsAPI::OpenGL;

  // Refresh rate used when entering exclusive Fullscreen
  // GLFW_DONT_CARE = use monitor's default
  int refresh_rate = GLFW_DONT_CARE;

  // Multisample anti-aliasing
  // 0 disabled, 4 = 4x MSAA, etc.
  int samples = 0;

  bool sRGB = false;
};

// A plain OS window + input handling. Deliberately knows nothing about how to
// render into itself: it hints GLFW with the right client API at creation time
// (WindowOpts::api) and exposes the native handle via GetNativeHandle(), and
// that's the entire extent of its rendering-API awareness. Building a graphics
// context on top (OpenGL::GraphicsContext) is the caller's job:
//
// auto window =
// Core::Window::Create(opts);
//
// auto context =
// Rendering::OpenGL::GraphicsContext::Create(window->GetNativeHandle());
// if(!context->Init())
//      EXIT:
//
//  window->PollEvents();
//
//  render into the default framebuffer
//  context->SwapBuffers(); // NOT window->SwapBuffers() - there isn't one
//
//  Vulkan doesn't have a "swap buffers" concept on the window at all (it
//  presents via a swapchain the app owns), so that call intentionally lives on
//  the context, not here
//
class Window {
public:
  using EventCallbackFn = std::function<void(Event &)>;

  explicit Window(const WindowOpts &opts);
  ~Window();

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(Window &&) = delete;

  static Scope<Window> Create(WindowOpts opts) {
    return std::make_unique<Window>(opts);
  }

  void PollEvents() { glfwPollEvents(); }
  bool ShouldClose() const { return glfwWindowShouldClose(m_WindowHandle); }
  void Close() { glfwSetWindowShouldClose(m_WindowHandle, GLFW_TRUE); }
  WindowMode GetWindowMode() const { return m_WindowMode; }

  void SetWindowMode(WindowMode mode);
  void ToggleFullscreen();

  bool IsMinimized() const {
    return glfwGetWindowAttrib(m_WindowHandle, GLFW_ICONIFIED) == GLFW_TRUE;
  }

  bool IsFocused() const {
    return glfwGetWindowAttrib(m_WindowHandle, GLFW_FOCUSED) == GLFW_TRUE;
  }

  void ShowCursor() {
    glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  void HideCursor() {
    glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  }

  void LockCursor() {
    glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  void UnlockCursor() {
    glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  i32 GetWidth() const { return m_Data.width; }
  i32 GetHeight() const { return m_Data.height; }

  std::pair<i32, i32> GetFramebufferSize() const {
    i32 width, height;
    glfwGetFramebufferSize(m_WindowHandle, &width, &height);
    return {width, height};
  }

  const std::string &GetTitle() const { return m_Title; }

  GLFWwindow *GetNativeHandle() const { return m_WindowHandle; }

  void SetEventCallback(EventCallbackFn callback) {
    m_EventCallback = std::move(callback);
  }

private:
  struct WindowData {
    i32 width, height;
  };

  struct WindowedState {
    i32 x, y;
    i32 width, height;
  };

  GLFWwindow *m_WindowHandle = nullptr;
  GLFWmonitor *m_Monitor = nullptr;
  i32 m_RefreshRate = GLFW_DONT_CARE;

  std::string m_Title;
  WindowMode m_WindowMode = WindowMode::Windowed;
  WindowData m_Data{};
  WindowedState m_WindowedState{};

  EventCallbackFn m_EventCallback;

  static u32 s_GLFWwindowCount;

  static void ApplyClientAPIHints(Rendering::GraphicsAPI api);

  void EnterFullscreen();
  void EnterBorderlessFullscreen();
  void EnterWindowed();
  GLFWmonitor *GetTargetMonitor() const;

  void FireEvent(Event &event) {
    if (m_EventCallback)
      m_EventCallback(event);
  }

  static void FramebufferResizeCallback(GLFWwindow *window, i32 width,
                                        i32 height);

  static void WindowSizeCallback(GLFWwindow *window, i32 width, i32 height);

  static void WindowPosCallback(GLFWwindow *window, i32 x, i32 y);

  static void WindowCloseCallback(GLFWwindow *window);

  static void WindowFocusCallback(GLFWwindow *window, i32 focued);

  static void WindowRefreshCallback(GLFWwindow *window);

  static void KeyCallback(GLFWwindow *window, i32 key, i32 scancode, i32 action,
                          i32 mode);

  static void CursorPosCallback(GLFWwindow *window, f64 x, f64 y);

  static void MouseButtonCallback(GLFWwindow *window, i32 button, i32 action,
                                  i32 mods);

  static void ScrollCallback(GLFWwindow *window, f64 offsetX, f64 offsetY);
};
}; // namespace Jupiter::Core
