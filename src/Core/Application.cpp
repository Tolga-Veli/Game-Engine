#include "Core/Application.hpp"

#include "Events/EventDispatcher.hpp"
#include "Events/WindowEvent.hpp"
#include "Rendering/Backend/IGraphicsContext.hpp"
#include "Rendering/Backend/RenderThread.hpp"

#include "Assets/AssetManager.hpp"

#include <chrono>

namespace Jupiter {

Application::Application(const ApplicationOpts &opts) {
  m_Window = Core::Window::Create(opts.window_opts);

  // Routes every callback through dispatcher so user can just define on event
  m_Window->SetEventCallback([this](Core::Event &event) { OnEvent(event); });
}

Application::~Application() {
  if (m_Running)
    Shutdown();
}

void Application::Run() {
  if (!Initialize())
    return;

  using Clock = std::chrono::steady_clock;
  auto lastTime = Clock::now();

  while (m_Running) {
    const auto now = Clock::now();
    const float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    ProcessEvents();

    if (!m_Running)
      break;

    OnUpdate(dt);

    if (!m_Running)
      break;

    OnRender(dt);

    if (m_Window->ShouldClose())
      Close();
  }

  Shutdown();
}

void Application::Close() noexcept { m_Running = false; }

bool Application::Initialize() {
  // Render thread owns the Renderer and context and must be responsible for the
  // creation of resources in asset managers
  if (!InitializeRenderThread())
    return false;

  OnInit();

  m_Running = true;
  return true;
}

bool Application::InitializeRenderThread() {
  Rendering::RenderThreadHooks hooks;

  hooks.Init = [this]() {
    m_GraphicsContext =
        Rendering::RHI::IGraphicsContext::Create(m_Window->GetNativeHandle());

    if (!m_GraphicsContext || !m_GraphicsContext->Init()) {
      LOG_FATAL("Application: graphics context failed to initialize");
      if (m_GraphicsContext) {
        m_GraphicsContext->Shutdown();
        m_GraphicsContext.reset();
      }
      return false;
    }

    Rendering::Renderer::Init();
    auto &assets = Assets::AssetManager::GetInstance();
    assets.SetRendererWakeup([]() { Rendering::Renderer::Wake(); });
    assets.Init();

    OnRenderInit();
    return true;
  };

  hooks.OnFrame = [this]() {
    Assets::AssetManager::GetInstance().ProcessRenderTasks();
    Assets::AssetManager::GetInstance().PollHotReload();
  };

  hooks.Present = [this]() { m_GraphicsContext->OnFrame(); };

  hooks.Shutdown = [this]() {
    OnRenderShutdown();
    Assets::AssetManager::GetInstance().Shutdown();
    Rendering::Renderer::Shutdown();
    m_GraphicsContext->Shutdown();
    m_GraphicsContext.reset();
  };

  return m_RenderThread.Start(std::move(hooks));
}

void Application::Shutdown() {
  OnShutdown();
  m_RenderThread.Stop();
  m_Running = false;
}

void Application::ProcessEvents() { m_Window->PollEvents(); }

void Application::OnEvent(Core::Event &event) {
  Core::EventDispatcher dispatcher(event);

  dispatcher.Dispatch<Core::WindowCloseEvent>([this](Core::WindowCloseEvent &) {
    Close();
    return true;
  });
}

} // namespace Jupiter
