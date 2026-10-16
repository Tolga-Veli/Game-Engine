#include "Core/Application.hpp"
#include "Rendering/RendererAPI.hpp"

#include "PlanetRenderer.hpp"
#include "Simulation.hpp"

#include "Events/EventDispatcher.hpp"
#include "Events/MouseEvent.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdlib>
#include <cstdio>
#include <glm/ext/matrix_clip_space.hpp>
#include <memory>
#include <mutex>

using namespace Jupiter;

class PlanetSim final : public Application {
public:
  PlanetSim() = default;

  void OnRenderInit() override {
    Rendering::RendererAPI::SetVSync(Rendering::RHI::VSyncMode::Off);
    m_Renderer = PlanetRenderer(kPlanetCount);

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

#ifdef WIN32
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

    ImGui::StyleColorsDark();

    constexpr float scale = 1.25f;
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;
    io.ConfigDpiScaleFonts = true;

    ImGui_ImplOpenGL3_Init();
    ImGui_ImplOpenGL3_CreateDeviceObjects();
  }

  void OnInit() override {
    std::scoped_lock lock(m_ImGuiMutex);
    ImGui_ImplGlfw_InitForOpenGL(GetWindow().GetNativeHandle(), true);
  }

  void OnShutdown() override {
    std::scoped_lock lock(m_ImGuiMutex);
    if (ImGui::GetCurrentContext() && ImGui::GetIO().BackendPlatformUserData)
      ImGui_ImplGlfw_Shutdown();
  }

  void ProcessEvents() override {
    std::scoped_lock lock(m_ImGuiMutex);
    Application::ProcessEvents();
  }

  void OnUpdate(float dt) override {
    if (m_LeftMouseHeld) {
      m_SpawnTimer -= dt;
      if (m_SpawnTimer <= 0.0f) {
        const auto [framebufferWidth, framebufferHeight] = GetWindow().GetFramebufferSize();
        const auto [windowWidth, windowHeight] = GetWindow().GetWindowSize();

        const glm::vec2 position{float(m_CursorPosition.x) * float(framebufferWidth) / float(windowWidth),
                                 float(framebufferHeight) -
                                     float(m_CursorPosition.y) * float(framebufferHeight) / float(windowHeight)};

        m_Simulation.SpawnPlanet(position, 20.0f);
        m_SpawnTimer = kSpawnInterval;
      }
    }

    m_Simulation.Update();
  }

  void OnRender(float) override {
    const auto [width, height] = GetWindow().GetFramebufferSize();
    if (width <= 0 || height <= 0)
      return;

    if (width != m_Width || height != m_Height) {
      m_Simulation.SetBounds(float(width), float(height));
      m_Width = width;
      m_Height = height;
    }

    // Draw data and font textures remain owned by ImGui. Let the previous
    // overlay finish before NewFrame() can reuse or resize them.
    if (m_GUICompletion.valid()) {
      while (m_GUICompletion.wait_for(std::chrono::milliseconds{1}) != std::future_status::ready)
        if (Rendering::RendererAPI::IsStopping())
          return;

      if (Rendering::RendererAPI::IsStopping())
        return;

      m_GUICompletion.get();
    }

    ImDrawData *guiFrame;
    {
      std::scoped_lock lock(m_ImGuiMutex);
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      m_ImGuiWantsMouse = ImGui::GetIO().WantCaptureMouse;

      ImGui::SetNextWindowSize(ImVec2(340.0f, 0.0f), ImGuiCond_FirstUseEver);
      ImGui::Begin("Planet controls");
      ImGui::ColorEdit3("Background", &m_BackgroundColor.x);
      ImGui::SeparatorText("Shader");
      ImGui::SliderFloat("Radius scale", &m_RadiusScale, 0.25f, 4.0f, "%.2f");
      ImGui::SliderFloat("Max speed", &m_MaxSpeed, 1.0f, 200.0f, "%.1f");
      ImGui::SliderFloat("Glow intensity", &m_GlowIntensity, 0.0f, 3.0f, "%.2f");
      ImGui::ColorEdit3("Color tint", &m_ColorTint.x);
      ImGui::Separator();
      ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
      ImGui::Separator();
      ImGui::Text("%zu Bodies alive", m_Simulation.GetPlanets().size());
      ImGui::End();

      ImGui::Render();
      guiFrame = ImGui::GetDrawData();
    }

    m_Renderer.SetRadiusScale(m_RadiusScale);
    m_Renderer.SetMaxSpeed(m_MaxSpeed);
    m_Renderer.SetGlowIntensity(m_GlowIntensity);
    m_Renderer.SetColorTint({m_ColorTint.x, m_ColorTint.y, m_ColorTint.z});

    Rendering::SceneData scene{.viewProjection = glm::mat4(1.0f),
                               .framebufferSize = {width, height},
                               .clearColor = {m_BackgroundColor.x, m_BackgroundColor.y, m_BackgroundColor.z, 1.0f}};

    if (!Rendering::RendererAPI::BeginFrame(scene))
      return;

    const auto &p = m_Simulation.GetPlanets();
    const auto projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

    m_Renderer.Draw(p.positions, p.velocities, p.radii, projection);

    auto completion = std::make_shared<std::promise<void>>();
    m_GUICompletion = completion->get_future();
    if (!Rendering::RendererAPI::SubmitOverlay([this, guiFrame, completion]() {
          std::scoped_lock lock(m_ImGuiMutex);
          ImGui_ImplOpenGL3_RenderDrawData(guiFrame);
          completion->set_value();
        }))
      return;

    Rendering::RendererAPI::EndFrame();
  }

  void OnRenderShutdown() override {
    std::scoped_lock lock(m_ImGuiMutex);
    if (!ImGui::GetCurrentContext())
      return;
    if (ImGui::GetIO().BackendRendererUserData)
      ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
  }

  void OnEvent(Core::Event &event) override {
    Application::OnEvent(event);

    Core::EventDispatcher dispatcher(event);
    dispatcher.Dispatch<Core::MouseMovedEvent>([this](Core::MouseMovedEvent &mouseEvent) {
      m_CursorPosition = {mouseEvent.GetX(), mouseEvent.GetY()};
      return false;
    });

    dispatcher.Dispatch<Core::MouseButtonPressedEvent>([this](Core::MouseButtonPressedEvent &mouseEvent) {
      if (mouseEvent.GetButton() == Core::MouseButton::Left && !m_ImGuiWantsMouse) {
        glfwGetCursorPos(GetWindow().GetNativeHandle(), &m_CursorPosition.x, &m_CursorPosition.y);
        m_LeftMouseHeld = true;
        m_SpawnTimer = 0.0f;
      }
      return false;
    });

    dispatcher.Dispatch<Core::MouseButtonReleasedEvent>([this](Core::MouseButtonReleasedEvent &mouseEvent) {
      if (mouseEvent.GetButton() == Core::MouseButton::Left)
        m_LeftMouseHeld = false;
      return false;
    });
  }

private:
  static constexpr u32 kPlanetCount{1000};
  static constexpr float kSpawnInterval{0.05f};
  PlanetSimulation m_Simulation{kPlanetCount};

  int m_Width{1280}, m_Height{720};
  PlanetRenderer m_Renderer;

  std::mutex m_ImGuiMutex;
  std::future<void> m_GUICompletion;
  glm::dvec2 m_CursorPosition{0.0};
  float m_SpawnTimer{0.0f};
  bool m_ImGuiWantsMouse{false}, m_LeftMouseHeld{false};
  ImVec4 m_BackgroundColor{0.02f, 0.025f, 0.04f, 1.0f}, m_ColorTint{1.0f, 1.0f, 1.0f, 1.0f};

  float m_RadiusScale{1.5f}, m_MaxSpeed{75.0f}, m_GlowIntensity{1.0f};
};

int main() {
  PlanetSim app;
  app.Run();
}
