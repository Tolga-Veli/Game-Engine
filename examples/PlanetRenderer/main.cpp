#include "Core/Application.hpp"
#include "PlanetRenderer.hpp"
#include "Simulation.hpp"
#include "Profiling/FpsCounter.hpp"
#include "Rendering/TextRenderer.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <string>

class PlanetApp final : public Jupiter::Application {
public:
  void OnRenderInit() override {
    GetGraphicsContext().SetVSync(Jupiter::Rendering::RHI::VSyncMode::Off);
    m_Renderer = PlanetRenderer(kPlanetCount);
    m_Text.Init();

    // Light mode
    // glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    //
    // Dark mode
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  }

  void OnUpdate(float) override { m_Simulation.Update(); }

  void OnRender(float dt) override {
    const auto [width, height] = GetWindow().GetFramebufferSize();
    if (width <= 0 || height <= 0)
      return;

    if (width != m_Width || height != m_Height) {
      m_Simulation.SetBounds(float(width), float(height));
      m_Width = width;
      m_Height = height;
    }

    Jupiter::Rendering::SceneData scene{
        .viewProjection = glm::mat4(1.0f),
        .framebufferSize = {width, height}};
    Jupiter::Rendering::Renderer::BeginFrame(scene);

    const auto &p = m_Simulation.GetPlanets();
    const auto projection = glm::ortho(0.0f, float(width), 0.0f, float(height));

    // m_Renderer.SetColorTint({1.0f, 1.0f, 1.0f});
    m_Renderer.Draw(p.positions, p.velocities, p.radii, projection);

    const int fps = static_cast<int>(m_Fps.Tick(dt) + 0.5f);
    m_Text.DrawText("FPS: " + std::to_string(fps),
                    {12.0f, float(height) - 30.0f}, {0.9f, 0.9f, 0.9f},
                    {width, height});

    Jupiter::Rendering::Renderer::EndFrame();
  }

private:
  static constexpr Jupiter::u32 kPlanetCount{350};
  PlanetSimulation m_Simulation{kPlanetCount};

  PlanetRenderer m_Renderer;
  Jupiter::Rendering::TextRenderer m_Text;

  Jupiter::Profiling::FpsCounter m_Fps;
  int m_Width{1280}, m_Height{720};
};

int main() {
  PlanetApp app;
  app.Run();
}
