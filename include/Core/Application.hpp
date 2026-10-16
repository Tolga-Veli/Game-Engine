#pragma once

#include "Rendering/Backend/RenderThread.hpp"
#include "Window.hpp"

#include <glm/glm.hpp>

namespace Jupiter {

class Application {
public:
  explicit Application(const Core::WindowOpts &opts = {});
  virtual ~Application();

  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  // Reports runtime failures to the caller after stopping the render thread
  // and releasing its resources.
  virtual void Run() final;
  void Close() noexcept;

protected:
  virtual void OnInit() {}
  virtual void OnShutdown() {}

  virtual void OnRenderInit() {}
  virtual void OnRenderShutdown() {}

  virtual void OnUpdate(float) {}
  virtual void OnRender(float) {}

  virtual void OnEvent(Core::Event &);
  virtual void ProcessEvents();

  Core::Window &GetWindow() noexcept { return m_Window; }
  const Core::Window &GetWindow() const noexcept { return m_Window; }

  bool IsRunning() const noexcept { return m_Running.load(std::memory_order_acquire); }

private:
  Core::Window m_Window;
  Rendering::RenderThread m_RenderThread;
  std::atomic<bool> m_Running{false};
  bool m_Initialized{false};

  [[nodiscard]] bool Init();
  [[nodiscard]] bool InitRenderThread();

  void Shutdown();
};
}; // namespace Jupiter
