#pragma once

#include "Utils.hpp"
#include "Window.hpp"

#include "Rendering/Backend/RenderThread.hpp"
#include "Rendering/Backend/IGraphicsContext.hpp"

#include "Assets/Traits/ShaderTraits.hpp"
#include "Assets/Traits/TextureTraits.hpp"
#include "Assets/Traits/FontTraits.hpp"
#include "Assets/Traits/MaterialTraits.hpp"
#include "Assets/Traits/MeshTraits.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace Jupiter {

struct ApplicationOpts {
  Core::WindowOpts window_opts{};
};

class Application {
public:
  explicit Application(const ApplicationOpts &opts = {});
  virtual ~Application();

  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

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

  Core::Window &GetWindow() noexcept { return *m_Window; }
  const Core::Window &GetWindow() const noexcept { return *m_Window; }

  Rendering::RHI::IGraphicsContext &GetGraphicsContext() noexcept {
    return *m_GraphicsContext;
  }

  bool IsRunning() const noexcept { return m_Running; }

private:
  Scope<Core::Window> m_Window;
  Scope<Rendering::RHI::IGraphicsContext> m_GraphicsContext;
  Rendering::RenderThread m_RenderThread;

  bool m_Running{false};

  [[nodiscard]] bool Initialize();
  [[nodiscard]] bool InitializeRenderThread();
  void Shutdown();
  void ProcessEvents();
};
}; // namespace Jupiter
