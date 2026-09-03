#include "Core/Application.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/Backend/SceneData.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Renderer.hpp"

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <cmath>
#include <string_view>

using namespace Jupiter;

class MyApp final : public Application {
public:
  explicit MyApp(bool smokeTest = false) : m_SmokeTest(smokeTest) {}

  void OnRenderInit() override {
    GetGraphicsContext().SetVSync(Rendering::RHI::VSyncMode::Off);

    shaderHandle =
        Assets::AssetManager::Get<Rendering::Shader>("Shaders/rot_cube.glsl");

    CORE_ASSERT(shaderHandle.IsValid(), "Failed to load rotating cube shader");

    Rendering::RHI::BufferLayout meshLayout{
        {Rendering::RHI::ShaderDataType::Float3, "aPos", 0}};

    constexpr float vertices[] = {// front
                                  -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
                                  0.5f, 0.5f, -0.5f, 0.5f, 0.5f,

                                  // back
                                  -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
                                  0.5f, -0.5f, -0.5f, 0.5f, -0.5f};

    constexpr u32 indices[] = {// front
                               0, 1, 2, 2, 3, 0,

                               // right
                               1, 5, 6, 6, 2, 1,

                               // back
                               5, 4, 7, 7, 6, 5,

                               // left
                               4, 0, 3, 3, 7, 4,

                               // top
                               3, 2, 6, 6, 7, 3,

                               // bottom
                               4, 5, 1, 1, 0, 4};

    meshHandle = Assets::AssetManager::Create<Rendering::Mesh>(
        "Cube", [vertices, indices, meshLayout]() {
          return Rendering::RHI::IMesh::Create(std::span<const float>(vertices),
                                               std::span<const u32>(indices),
                                               meshLayout);
        });

    materialHandle = Assets::AssetManager::Create<Rendering::Material>(
        "RGB", []() { return Rendering::RHI::IMaterial::Create(); });
  }

  void OnRender(float dt) override {
    m_Time += dt;

    auto shader =
        Assets::AssetManager::Resolve<Rendering::Shader>(shaderHandle);

    auto mesh = Assets::AssetManager::Resolve<Rendering::Mesh>(meshHandle);

    auto material =
        Assets::AssetManager::Resolve<Rendering::Material>(materialHandle);

    if (!shader || !mesh || !material)
      return;

    const float redv = std::sin(m_Time) * 0.5f + 0.5f,
                greenv = std::sin(m_Time * 0.7f) * 0.5f + 0.5f,
                bluev = std::sin(m_Time * 1.3f) * 0.5f + 0.5f;

    const glm::vec4 color{redv, greenv, bluev, 1.0f};

    const auto [width, height] = GetWindow().GetFramebufferSize();
    if (width <= 0 || height <= 0)
      return;

    const float aspect = static_cast<float>(width) / height;

    const glm::mat4 view = glm::translate(glm::mat4(1.0f),
                                          glm::vec3{0.0f, 0.0f, -3.0f}),
                    proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f,
                                            100.0f);

    const glm::mat4 model = glm::rotate(
        glm::mat4(1.0f), m_Time, glm::normalize(glm::vec3{0.5f, 1.0f, 0.25f}));

    Rendering::SceneData scene{.framebufferSize = {width, height}};
    scene.viewProjection = proj * view;
    Rendering::Renderer::BeginFrame(scene);

    material->SetShader(shaderHandle);
    material->Set("uColor", color);

    Rendering::Renderer::Submit(
        Rendering::RHI::RenderCommand{mesh, material, model});
    Rendering::Renderer::EndFrame();

    if (m_SmokeTest && ++m_FrameCount >= 120)
      Close();
  }

private:
  Assets::AssetHandle<Rendering::Shader> shaderHandle;
  Assets::AssetHandle<Rendering::Mesh> meshHandle;
  Assets::AssetHandle<Rendering::Material> materialHandle;

  float m_Time{0.0f};
  u32 m_FrameCount{0};
  bool m_SmokeTest{false};
};

int main(int argc, char **argv) {
  const bool smokeTest =
      argc == 2 && std::string_view(argv[1]) == "--smoke-test";

  MyApp app{smokeTest};
  app.Run();
}
