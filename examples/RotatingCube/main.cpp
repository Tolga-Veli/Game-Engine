#include "Core/Application.hpp"
#include "Assets/AssetManager.hpp"
#include "Rendering/Backend/SceneData.hpp"
#include "Rendering/RendererAPI.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string_view>

using namespace Jupiter;

class MyApp final : public Application {
public:
  MyApp() = default;

  void OnRenderInit() override {
    Rendering::RendererAPI::SetVSync(Rendering::RHI::VSyncMode::Off);
    m_Shader = Assets::AssetManager::Load<Rendering::Shader>("Shaders/rot_cube.glsl");

    constexpr float vertices[] = {-0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
                                  -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
    constexpr u32 indices[] = {0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 5, 4, 7, 7, 6, 5,
                               4, 0, 3, 3, 7, 4, 3, 2, 6, 6, 7, 3, 4, 5, 1, 1, 0, 4};

    Rendering::RHI::MeshDesc desc;
    desc.layout = {{Rendering::RHI::DataType::Float3, "aPos", 0}};
    desc.vertices = {Rendering::RHI::BufferType::Vertex, Rendering::RHI::BufferUsage::Static,
                     Rendering::RHI::DataType::Float3, sizeof(vertices)};
    desc.indices = {Rendering::RHI::BufferType::Index, Rendering::RHI::BufferUsage::Static,
                    Rendering::RHI::DataType::UInt, sizeof(indices)};
    desc.vertexCap = 8;
    desc.indexCap = 36;
    desc.vertexData = Storage{vertices};
    desc.indexData = Storage{indices};

    m_Mesh = Assets::AssetManager::Create<Rendering::Mesh>("Cube", desc);
    m_Material = Assets::AssetManager::Create<Rendering::Material>("RGB", {});
    Rendering::RendererAPI::SubmitRenderTask([material = m_Material, shader = m_Shader](auto &) {
      if (auto value = Assets::AssetManager::Resolve<Rendering::Material>(material))
        value->SetShader(shader);
    });
  }

  void OnRender(float dt) override {
    m_Time += dt;
    const auto [width, height] = GetWindow().GetFramebufferSize();
    if (width <= 0 || height <= 0)
      return;

    const float aspect = static_cast<float>(width) / height;
    const glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, -3.0f});
    const glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
    const glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_Time, glm::normalize(glm::vec3{0.5f, 1.0f, 0.25f}));

    Rendering::SceneData scene{.viewProjection = projection * view, .framebufferSize = {width, height}};
    if (!Rendering::RendererAPI::BeginFrame(scene))
      return;
    Rendering::RHI::DrawCommand command{m_Mesh, m_Material, model};
    command.ApplyOverride("uColor", glm::vec4{std::sin(m_Time) * 0.5f + 0.5f, std::sin(m_Time * 0.7f) * 0.5f + 0.5f,
                                              std::sin(m_Time * 1.3f) * 0.5f + 0.5f, 1.0f});
    Rendering::RendererAPI::SubmitDrawCommand(std::move(command));
    Rendering::RendererAPI::EndFrame();
  }

private:
  Assets::AssetHandle<Rendering::Shader> m_Shader;
  Assets::AssetHandle<Rendering::Mesh> m_Mesh;
  Assets::AssetHandle<Rendering::Material> m_Material;
  float m_Time{};
};

int main() {
  try {
    MyApp app;
    app.Run();
  } catch (...) {}
}
