#pragma once

#include "Core/Utils.hpp"
#include "Rendering/GraphicsAPI.hpp"
#include "Events/Event.hpp"

struct GLFWwindow;

namespace Jupiter::Core {
enum class WindowMode : u8 { Windowed, Fullscreen, BorderlessFullscreen };

struct WindowOpts {
  std::string_view title = "Jupiter Engine";
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

  Rendering::GraphicsAPI api = Rendering::GraphicsAPI::OpenGL;

  // Refresh rate used when entering exclusive Fullscreen
  // GLFW_DONT_CARE = use monitor's default
  i32 refresh_rate = -1;

  // Multisample anti-aliasing
  // 0 disabled, 4 = 4x MSAA, etc.
  i32 samples = 0;
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

namespace detail {
class IWindow {
public:
  using EventCallbackFn = std::function<void(Event &)>;

  virtual ~IWindow() = default;
  virtual void SetEventCallback(EventCallbackFn callback) = 0;

  virtual void PollEvents() noexcept = 0;
  virtual bool ShouldClose() const noexcept = 0;
  virtual void Close() noexcept = 0;

  virtual WindowMode GetWindowMode() const noexcept = 0;

  virtual void SetWindowMode(WindowMode mode) noexcept = 0;
  virtual void ToggleFullscreen() noexcept = 0;

  virtual bool IsMinimized() const noexcept = 0;
  virtual bool IsFocused() const noexcept = 0;
  virtual void ShowCursor() noexcept = 0;
  virtual void HideCursor() noexcept = 0;
  virtual void LockCursor() noexcept = 0;
  virtual void UnlockCursor() noexcept = 0;

  virtual std::pair<i32, i32> GetWindowSize() const noexcept = 0;
  virtual std::pair<i32, i32> GetFramebufferSize() const noexcept = 0;

  virtual std::string_view GetTitle() const noexcept = 0;

  virtual GLFWwindow *GetNativeHandle() const noexcept = 0;

}; // namespace Jupiter::Core
} // namespace detail
} // namespace Jupiter::Core
