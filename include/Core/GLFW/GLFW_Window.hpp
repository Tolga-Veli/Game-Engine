#pragma once

#include "Core/Backend/IWindow.hpp"

namespace Jupiter::Core::detail {
class GLFW_Window final : public detail::IWindow {
public:
  GLFW_Window(const WindowOpts &opts);
  ~GLFW_Window() override;

  void PollEvents() noexcept override { glfwPollEvents(); }
  bool ShouldClose() const noexcept override { return glfwWindowShouldClose(m_WindowHandle); }
  void Close() noexcept override { glfwSetWindowShouldClose(m_WindowHandle, GLFW_TRUE); }

  bool IsMinimized() const noexcept override {
    return glfwGetWindowAttrib(m_WindowHandle, GLFW_ICONIFIED) == GLFW_TRUE;
  }
  bool IsFocused() const noexcept override { return glfwGetWindowAttrib(m_WindowHandle, GLFW_FOCUSED) == GLFW_TRUE; }
  void ShowCursor() noexcept override { glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
  void HideCursor() noexcept override { glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); }
  void LockCursor() noexcept override { glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
  void UnlockCursor() noexcept override { glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

  std::pair<i32, i32> GetWindowSize() const noexcept override {
    i32 width, height;
    glfwGetWindowSize(m_WindowHandle, &width, &height);
    return {width, height};
  }

  std::pair<i32, i32> GetFramebufferSize() const noexcept override {
    i32 width, height;
    glfwGetFramebufferSize(m_WindowHandle, &width, &height);
    return {width, height};
  }

  std::string_view GetTitle() const noexcept override { return m_Title; }

  WindowMode GetWindowMode() const noexcept override { return m_WindowMode; }
  void SetWindowMode(WindowMode mode) noexcept override;
  void ToggleFullscreen() noexcept override;

  void EnterWindowed();
  void EnterFullscreen();
  void EnterBorderlessFullscreen();

  void FireEvent(Event &event) {
    if (m_EventCallback)
      m_EventCallback(event);
  }

  GLFWwindow *GetNativeHandle() const noexcept override { return m_WindowHandle; }

private:
  GLFWwindow *m_WindowHandle{};
  WindowMode m_WindowMode;
  i32 m_RefreshRate;
  std::string_view m_Title;

  EventCallbackFn m_EventCallback;

private:
  void ApplyClientAPIHints(Rendering::GraphicsAPI api) const;

  static void GLFWErrorCallback(i32 error, const char *desc) { LOG_ERROR("GLFW error ({}): {}", error, desc); }

  static void FramebufferResizeCallback(GLFWwindow *window, i32 width, i32 height);
  static void WindowSizeCallback(GLFWwindow *window, i32 width, i32 height);
  static void WindowPosCallback(GLFWwindow *window, i32 x, i32 y);
  static void WindowCloseCallback(GLFWwindow *window);
  static void WindowFocusCallback(GLFWwindow *window, i32 focued);
  static void WindowRefreshCallback(GLFWwindow *window);
  static void KeyCallback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mode);
  static void CursorPosCallback(GLFWwindow *window, f64 x, f64 y);
  static void MouseButtonCallback(GLFWwindow *window, i32 button, i32 action, i32 mods);
  static void ScrollCallback(GLFWwindow *window, f64 offsetX, f64 offsetY);
};
} // namespace Jupiter::Core::detail
