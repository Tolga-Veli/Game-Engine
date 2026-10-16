#pragma once

#include "Events/Event.hpp"
#include "Core/GLFW/GLFW_Window.hpp"

#include <functional>
#include <memory>
#include <utility>

namespace Jupiter::Core {
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

  explicit Window(const WindowOpts &opts) : m_WindowImpl(CreateScope<detail::GLFW_Window>(opts)), m_API(opts.api) {}
  ~Window() = default;

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(Window &&) = delete;

  void PollEvents() noexcept { m_WindowImpl->PollEvents(); }
  bool ShouldClose() const noexcept { return m_WindowImpl->ShouldClose(); }
  void Close() noexcept { m_WindowImpl->Close(); }

  WindowMode GetWindowMode() const noexcept { return m_WindowImpl->GetWindowMode(); }
  void SetWindowMode(WindowMode mode) const noexcept { m_WindowImpl->SetWindowMode(mode); }
  void ToggleFullscreen() const noexcept { m_WindowImpl->ToggleFullscreen(); }

  bool IsMinimized() const noexcept { return m_WindowImpl->IsMinimized(); }
  bool IsFocused() const noexcept { return m_WindowImpl->IsFocused(); }
  void ShowCursor() noexcept { return m_WindowImpl->ShowCursor(); }
  void HideCursor() noexcept { return m_WindowImpl->HideCursor(); }
  void LockCursor() noexcept { return m_WindowImpl->LockCursor(); }
  void UnlockCursor() noexcept { return m_WindowImpl->UnlockCursor(); }

  std::pair<i32, i32> GetWindowSize() const noexcept { return m_WindowImpl->GetWindowSize(); }
  std::pair<i32, i32> GetFramebufferSize() const noexcept { return m_WindowImpl->GetFramebufferSize(); }

  std::string_view GetTitle() const noexcept { return m_WindowImpl->GetTitle(); }

  void SetEventCallback(EventCallbackFn callback) { m_WindowImpl->SetEventCallback(std::move(callback)); }

  GLFWwindow *GetNativeHandle() const noexcept { return m_WindowImpl->GetNativeHandle(); }
  Rendering::GraphicsAPI GetGraphicsAPI() const noexcept { return m_API; }

private:
  Scope<detail::IWindow> m_WindowImpl;
  Rendering::GraphicsAPI m_API;
};
}; // namespace Jupiter::Core
