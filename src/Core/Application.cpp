#include "Application.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Traits.hpp"
#include "Core/Utils.hpp"
#include "Events/EventDispatcher.hpp"
#include "Profiling/FpsCounter.hpp"
#include "Rendering/Backend/IGraphicsContext.hpp"
#include "Rendering/Renderer.hpp"

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Jupiter {

Application::Application() { Init(); };
Application::~Application() { Shutdown(); }

void Application::OnEvent(Core::Event &event) {
  Core::EventDispatcher dispatcher(event);

  dispatcher.Dispatch<Core::WindowCloseEvent>(J_BIND_EVENT_FN(OnWindowClose));
  dispatcher.Dispatch<Core::WindowResizeEvent>(J_BIND_EVENT_FN(OnWindowResize));
  dispatcher.Dispatch<Core::WindowFramebufferResizeEvent>(
      J_BIND_EVENT_FN(OnWindowFramebufferResize));
  dispatcher.Dispatch<Core::KeyPressedEvent>(J_BIND_EVENT_FN(OnKeyPressed));
}

bool Application::OnWindowClose(Core::WindowCloseEvent &event) {
  (void)event;
  m_Running = false;
  return true;
}

bool Application::OnWindowResize(Core::WindowResizeEvent &event) {
  m_Simulation.SetBounds(static_cast<float>(event.GetWidth()),
                         static_cast<float>(event.GetHeight()));
  return true;
}

bool Application::OnWindowFramebufferResize(
    Core::WindowFramebufferResizeEvent &event) {
  glViewport(0, 0, event.GetWidth(), event.GetHeight());
  return true;
}

bool Application::OnKeyPressed(Core::KeyPressedEvent &event) {
  // F11 fullscreen toggle lives here now instead of inside Window - Window
  // just translates GLFW into events, it doesn't get to have input policy.
  if (event.GetKeyCode() == Core::KeyCode::F11 && !event.IsRepeat()) {
    m_Window->ToggleFullscreen();
    return true;
  }

  return false;
}

void Application::Init() {
  m_Window = Core::Window::Create();

  m_GraphicsContext =
      Rendering::IGraphicsContext::Create(m_Window->GetNativeHandle());

  if (!m_GraphicsContext->Init()) {
    LOG_ERROR("Application: graphics context failed to initialize");
    return;
  }

  m_GraphicsContext->SetVSync(Rendering::VSyncMode::Off);

  m_Window->SetEventCallback([this](Core::Event &event) { OnEvent(event); });

  Rendering::Renderer::Init();
  Assets::AssetManager::GetInstance().Init();

  m_TextRenderer.SetMaxCharacters(256);
  m_TextRenderer.Init();

  m_FontHandle = Assets::AssetManager::Get<Rendering::Font>(
      "Fonts/Montserrat-Regular.ttf", Assets::FontLoadOptions{24.0f});

  m_PlanetRenderer.SetMaxBodies(k_BodyCount);
  m_PlanetRenderer.Init();

  m_Simulation.SetBodyCount(k_BodyCount);
  m_Simulation.SetBounds(static_cast<float>(m_Window->GetWidth()),
                         static_cast<float>(m_Window->GetHeight()));
  m_Simulation.Init();

  m_Running = true;
}

void Application::Shutdown() {
  m_Running = false;
  Rendering::Renderer::Shutdown();
  Assets::AssetManager::GetInstance().Shutdown();
}

void Application::OnUpdate() {
  m_Simulation.Update();
  auto &planets = m_Simulation.GetPlanets();

  m_Positions = planets.positions;
  m_Radii = planets.radii;
  m_Velocities = planets.velocities;
}

void Application::OnRender(float fps) {
  Rendering::Renderer::Clear();

  glm::mat4 screenProj =
      glm::ortho(0.0f, static_cast<float>(m_Window->GetWidth()),
                 static_cast<float>(m_Window->GetHeight()), 0.0f, -1.0f, 1.0f);

  Rendering::Renderer::BeginScene(Rendering::SceneData{screenProj});

  m_PlanetRenderer.Draw(m_Positions, m_Velocities, m_Radii, screenProj);

  auto text = std::format("N = {}  FPS = {:.0f}  {:.2f} ms",
                          m_Simulation.GetPlanets().size(), fps, 1000.0f / fps);

  m_TextRenderer.DrawText(
      Assets::AssetManager::GetInstance().Resolve(m_FontHandle), text,
      glm::vec2(35.0f, 50.0f), Core::Colors::White, screenProj);

  Rendering::Renderer::Flush();
  Rendering::Renderer::EndScene();
}

void Application::Run() {
  using clock = std::chrono::steady_clock;
  auto last_time = clock::now();
  Profiling::FpsCounter fpsCounter;

  Rendering::Renderer::ClearColor({0.1f, 0.1f, 0.1f, 1.0f});
  while (m_Running) {
    const auto now = clock::now();
    const float dt = std::chrono::duration<float>(now - last_time).count();
    last_time = now;

    const float fps = fpsCounter.Tick(dt);

    Assets::AssetManager::GetInstance().PollHotReload();
    m_Window->PollEvents();

    OnUpdate();
    OnRender(fps);

    m_GraphicsContext->SwapBuffers();
    if (m_Window->ShouldClose())
      m_Running = false;
  }
}
}; // namespace Jupiter
