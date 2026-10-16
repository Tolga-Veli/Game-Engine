#include "Core/Application.hpp"

#include "Events/EventDispatcher.hpp"
#include "Events/WindowEvent.hpp"
#include "Rendering/Backend/RenderThread.hpp"

#include "Assets/AssetManager.hpp"

#include <chrono>
#include <exception>

namespace Jupiter {

Application::Application(const Core::WindowOpts &opts) : m_Window(opts) {
  m_Window.SetEventCallback([this](Core::Event &event) { OnEvent(event); });
}

Application::~Application() {
  try {
    if (m_Initialized)
      Shutdown();
  } catch (...) {}
}

void Application::Run() {
  try {
    if (!Init())
      return;

    using Clock = std::chrono::steady_clock;
    auto lastTime = Clock::now();

    while (m_Running && !m_RenderThread.HasFailed()) {
      const auto now = Clock::now();
      const float dt = std::chrono::duration<float>(now - lastTime).count();
      lastTime = now;

      ProcessEvents();

      if (!m_Running || m_RenderThread.HasFailed())
        break;

      OnUpdate(dt);

      if (!m_Running || m_RenderThread.HasFailed())
        break;

      OnRender(dt);

      if (m_Window.ShouldClose())
        Close();
    }
  } catch (...) {
    const auto failure = std::current_exception();

    if (m_Initialized) {
      try {
        Shutdown();
      } catch (...) {}
    } else
      m_RenderThread.Stop();

    std::rethrow_exception(failure);
  }

  if (m_Initialized)
    Shutdown();

  m_RenderThread.RethrowIfFailed();
}

void Application::Close() noexcept { m_Running = false; }

bool Application::Init() {
  // Render thread owns the Renderer and context and must be responsible for the
  // creation of resources in asset managers
  if (!InitRenderThread())
    return false;

  m_Initialized = true;
  OnInit();

  m_Running = true;
  return true;
}

bool Application::InitRenderThread() {
  Rendering::RenderThreadHooks hooks;

  hooks.Init = [this]() {
    bool rendererAttempted = false, assetsInitialized = false, renderInitStarted = false;
    try {
      rendererAttempted = true;
      Rendering::RendererAPI::Init(m_Window.GetNativeHandle());

      auto &assets = Assets::AssetManager::GetInstance();
      assets.SetRendererWakeup([] { Rendering::RendererAPI::Wake(); });
      assets.Init();
      assetsInitialized = true;

      renderInitStarted = true;
      OnRenderInit();
      return true;
    } catch (...) {
      const auto failure = std::current_exception();
      if (renderInitStarted)
        try {
          OnRenderShutdown();
        } catch (...) {}

      if (assetsInitialized)
        try {
          Assets::AssetManager::GetInstance().Shutdown();
        } catch (...) {}

      if (rendererAttempted)
        try {
          Rendering::RendererAPI::Shutdown();
        } catch (...) {}

      std::rethrow_exception(failure);
    }
  };

  hooks.OnFrame = []() {
    auto &assets = Assets::AssetManager::GetInstance();
    assets.ProcessRenderTasks();
    assets.PollHotReload();
  };

  hooks.Shutdown = [this]() {
    std::exception_ptr failure;

    auto attempt = [&failure](auto &&op) {
      try {
        op();
      } catch (...) {
        if (!failure)
          failure = std::current_exception();
      }
    };

    attempt([this] { OnRenderShutdown(); });
    attempt([] { Assets::AssetManager::GetInstance().Shutdown(); });
    attempt([] { Rendering::RendererAPI::Shutdown(); });

    if (failure)
      std::rethrow_exception(failure);
  };

  return m_RenderThread.Start(std::move(hooks));
}

void Application::Shutdown() {
  m_Running = false;
  std::exception_ptr failure;

  try {
    OnShutdown();
  } catch (...) { failure = std::current_exception(); }

  m_RenderThread.Stop();
  m_Initialized = false;

  if (failure)
    std::rethrow_exception(failure);
}

void Application::ProcessEvents() { m_Window.PollEvents(); }

void Application::OnEvent(Core::Event &event) {
  Core::EventDispatcher dispatcher(event);

  dispatcher.Dispatch<Core::WindowCloseEvent>([this](Core::WindowCloseEvent &) -> bool {
    Close();
    return true;
  });
}

} // namespace Jupiter
