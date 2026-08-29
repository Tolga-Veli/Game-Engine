#pragma once

#include "Utils.hpp"
#include "Window.hpp"

#include "Rendering/Backend/IGraphicsContext.hpp"
#include "Rendering/TextRenderer.hpp"
#include "Rendering/RenderThread.hpp"

#include "Engine/Simulation.hpp"
#include "Engine/PlanetRenderer.hpp"

#include "Events/WindowEvent.hpp"
#include "Events/KeyboardEvent.hpp"

#include <chrono>
#include <glm/glm.hpp>
#include <memory>

namespace Jupiter {

class Application {
public:
  Application();
  ~Application();

  void Init();
  void Shutdown();

  static Scope<Application> Create() { return std::make_unique<Application>(); }

  void Run();
  void OnUpdate();
  void OnRender(float fps);

  void OnEvent(Core::Event &event);

private:
  static constexpr u32 k_BodyCount = 500;
  static constexpr std::chrono::milliseconds k_MinRefreshInterval{
      16}; // ~60Hz cap

  std::chrono::steady_clock::time_point m_LastRefreshRedraw =
      std::chrono::steady_clock::now();

  Scope<Core::Window> m_Window;
  Scope<Rendering::IGraphicsContext> m_GraphicsContext;

  Rendering::TextRenderer m_TextRenderer;
  Assets::AssetHandle<Rendering::Font> m_FontHandle;

  Engine::PlanetSimulation m_Simulation;
  Rendering::PlanetRenderer m_PlanetRenderer;

  std::span<const glm::vec2> m_Positions, m_Velocities;
  std::span<const float> m_Radii;

  bool m_Running;

  bool OnWindowClose(Core::WindowCloseEvent &e);
  bool OnWindowResize(Core::WindowResizeEvent &e);
  bool OnWindowFramebufferResize(Core::WindowFramebufferResizeEvent &e);
  bool OnKeyPressed(Core::KeyPressedEvent &e);
};
}; // namespace Jupiter
