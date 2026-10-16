#include "Core/Application.hpp"

#include "PlanetRenderer.hpp"
#include "Simulation.hpp"

#include "Events/EventDispatcher.hpp"
#include "Events/MouseEvent.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdlib>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <glm/ext/matrix_clip_space.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace detail {
// ImGui owns the data returned by GetDrawData() and reuses it on NewFrame().
// Keep an independent copy alive until the render thread has drawn the overlay.
struct ImGuiDrawDataSnapshot {
  explicit ImGuiDrawDataSnapshot(const ImDrawData &source) {
    drawData.Valid = source.Valid;
    drawData.FrameCount = source.FrameCount;
    drawData.DisplayPos = source.DisplayPos;
    drawData.DisplaySize = source.DisplaySize;
    drawData.FramebufferScale = source.FramebufferScale;
    drawData.OwnerViewport = source.OwnerViewport;
    drawData.Textures = source.Textures;

    drawLists.reserve(source.CmdLists.Size);
    for (const ImDrawList *drawList : source.CmdLists) {
      drawLists.emplace_back(drawList->CloneOutput());
      drawData.CmdLists.push_back(drawLists.back().get());
      drawData.TotalIdxCount += drawLists.back()->IdxBuffer.Size;
      drawData.TotalVtxCount += drawLists.back()->VtxBuffer.Size;
    }
    drawData.CmdListsCount = drawData.CmdLists.Size;
  }

  ImDrawData drawData;
  std::vector<std::unique_ptr<ImDrawList>> drawLists;
};

} // namespace detail

using namespace Jupiter;

class PlanetSim final : public Application {
public:
  void OnRenderInit() override {
    Rendering::RendererAPI::SetVSync(Rendering::RHI::VSyncMode::Off);
    m_Renderer = PlanetRenderer(kPlanetCount);

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

#ifdef WIN32
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

    ImGui::StyleColorsDark();

    constexpr float scale = 1.25f;
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;
    io.ConfigDpiScaleFonts = true;

    ImGui_ImplGlfw_InitForOpenGL(GetWindow().GetNativeHandle(), true);
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplOpenGL3_CreateDeviceObjects();
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
        m_Simulation.SpawnPlanet(position);

        m_SpawnTimer = kSpawnInterval;
      }
    }

    m_Simulation.Update();
  }

  void OnRender(float) override {
    const auto [width, height] = GetWindow().GetFramebufferSize();

    if (width != m_Width || height != m_Height) {
      m_Simulation.SetBounds(float(width), float(height));
      m_Width = width;
      m_Height = height;
    }

    std::shared_ptr<detail::ImGuiDrawDataSnapshot> guiFrame;
    {
      std::scoped_lock lock(m_ImGuiMutex);
      ImGui_ImplOpenGL3_NewFrame();
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
      guiFrame = std::make_shared<detail::ImGuiDrawDataSnapshot>(*ImGui::GetDrawData());
    }

    m_Renderer.SetRadiusScale(m_RadiusScale);
    m_Renderer.SetMaxSpeed(m_MaxSpeed);
    m_Renderer.SetGlowIntensity(m_GlowIntensity);
    m_Renderer.SetColorTint({m_ColorTint.x, m_ColorTint.y, m_ColorTint.z});

    Rendering::SceneData scene{.viewProjection = glm::mat4(1.0f),
                               .framebufferSize = {width, height},
                               .clearColor = {m_BackgroundColor.x, m_BackgroundColor.y, m_BackgroundColor.z, 1.0f}};

    Rendering::RendererAPI::BeginFrame(scene);

    const auto &p = m_Simulation.GetPlanets();
    const auto projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

    m_Renderer.Draw(p.positions, p.velocities, p.radii, projection);

    Rendering::RendererAPI::SubmitOverlay([this, guiFrame]() {
      std::scoped_lock lock(m_ImGuiMutex);
      ImGui_ImplOpenGL3_RenderDrawData(&guiFrame->drawData);
    });

    Rendering::RendererAPI::EndFrame();
  }

  void OnRenderShutdown() override {
    std::scoped_lock lock(m_ImGuiMutex);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
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
