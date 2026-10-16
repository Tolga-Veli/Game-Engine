#ifdef TEST_2
#include "Assets/AssetTraits.hpp"
#include "Core/Assert.hpp"
#include "Core/Utils.hpp"
#include "Core/Window.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Traits/ShaderTraits.hpp"
#include "Assets/Traits/MeshTraits.hpp"
#include "Assets/Traits/MaterialTraits.hpp"

#include "Rendering/Backend/BufferLayout.hpp"
#include "Rendering/Backend/IGraphicsContext.hpp"
#include "Rendering/Backend/IShader.hpp"
#include "Rendering/Backend/RenderCommand.hpp"
#include "Rendering/Backend/RenderThread.hpp"
#include "Rendering/Backend/ShaderTypes.hpp"
#include "Rendering/Shader.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include <chrono>
#include <glm/glm.hpp>

using namespace Jupiter;
using namespace Jupiter::Rendering;

// clang-format off
constexpr float vertices[] = {
                              // front 
                              -0.5f, -0.5f,  0.5f,  
                               0.5f, -0.5f,  0.5f,
                               0.5f,  0.5f,  0.5f,
                              -0.5f,  0.5f,  0.5f,
                              
                             // back
                              -0.5f, -0.5f, -0.5f,
                               0.5f, -0.5f, -0.5f,
                               0.5f,  0.5f, -0.5f,
                              -0.5f, 0.5f,  -0.5f};

constexpr u32 indices[] = {
  // front
  0,1,2,
  2,3,0,

  // right
  1,5,6,
  6,2,1,

  // back
  5,4,7,
  7,6,5,

  // left
  4,0,3,
  3,7,4,

  // top
  3,2,6,
  6,7,3,

  // bottom
  4,5,1,
  1,0,4
};
// clang-format on

int main() {
  auto window = Core::Window::Create();
  Scope<Rendering::RHI::IGraphicsContext> context;

  Assets::AssetHandle<Shader> shaderHandle;
  Assets::AssetHandle<Mesh> meshHandle;
  Assets::AssetHandle<Material> materialHandle;

  RenderThread renderThread;
  RenderThreadHooks hooks;

  hooks.Init = [&]() {
    context = RHI::IGraphicsContext::Create(window->GetNativeHandle());
    context->Init();
    context->SetVSync(RHI::VSyncMode::Off);

    Assets::AssetManager::GetInstance().Init();

    Renderer::Init();

    Rendering::RHI::IShader::ShaderSources sources{};
    sources[EnumToSize(RHI::ShaderType::Vertex)] = R"glsl(
      #version 330 core

      layout(location = 0) in vec3 aPos;

      uniform mat4 uViewProjection;
      uniform mat4 uModel;

      void main() {
        gl_Position = uViewProjection * uModel * vec4(aPos, 1.0);
      }
    )glsl";

    sources[EnumToSize(RHI::ShaderType::Fragment)] = R"glsl(
      #version 330 core

      out vec4 FragColor;

      uniform vec4 uColor;

      void main() {
        FragColor = uColor;
      }
    )glsl";

    shaderHandle =
        Assets::AssetManager::Create<Shader>("rotation_and_color", [sources]() {
          return RHI::IShader::Create(sources);
        });

    RHI::BufferLayout meshLayout{{RHI::ShaderDataType::Float3, "aPos", 0}};

    meshHandle = Assets::AssetManager::Create<Mesh>("cube", [meshLayout]() {
      auto impl = RHI::IMesh::Create(std::span<const float>(vertices),
                                     std::span<const u32>(indices), meshLayout);
      CORE_ASSERT(impl != nullptr, "failed to create IMesh");
      return impl;
    });

    materialHandle = Assets::AssetManager::Create<Material>("color", []() {
      auto impl = RHI::IMaterial::Create();
      CORE_ASSERT(impl != nullptr, "failed to create IMaterial");
      return impl;
    });

    Assets::AssetManager::GetInstance().ProcessRenderTasks();
  };

  hooks.OnFrame = [&]() {
    Assets::AssetManager::GetInstance().ProcessRenderTasks();
    auto mesh = Assets::AssetManager::Resolve<Mesh>(meshHandle);
    context->SwapBuffers();
  };

  hooks.Shutdown = [&]() {
    Assets::AssetManager::GetInstance().Shutdown();
    Renderer::Shutdown();
    context.reset();
  };

  renderThread.Start(std::move(hooks));

  using Clock = std::chrono::steady_clock;
  const auto start = Clock::now();

  std::this_thread::sleep_for(std::chrono::seconds{1});

  while (!window->ShouldClose()) {
    window->PollEvents();

    const float seconds =
        std::chrono::duration<float>(Clock::now() - start).count();

    const float redv = std::sin(seconds) * 0.5f + 0.5f,
                greenv = std::sin(seconds * 0.7f) * 0.5f + 0.5f,
                bluev = std::sin(seconds * 1.3f) * 0.5f + 0.5f;

    const glm::vec4 color{redv, greenv, bluev, 1.0f};

    const auto [width, height] = window->GetFramebufferSize();
    const float aspect = static_cast<float>(width) / height;
    const glm::mat4 view = glm::translate(glm::mat4(1.0f),
                                          glm::vec3{0.0f, 0.0f, -3.0f}),
                    proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f,
                                            100.0f);

    [[maybe_unused]] const glm::mat4 model = glm::rotate(
        glm::mat4(1.0f), seconds, glm::normalize(glm::vec3{0.5f, 1.0f, 0.25f}));

    SceneData scene{};
    scene.viewProjection = proj * view;

    Renderer::BeginFrame(scene);

    auto mesh = Assets::AssetManager::Resolve<Mesh>(meshHandle);
    auto material = Assets::AssetManager::Resolve<Material>(materialHandle);

    material->SetShader(shaderHandle);
    material->Set("uColor", color);
    Renderer::Submit(RHI::RenderCommand{mesh, material, model});

    Renderer::EndFrame();
  }

  renderThread.Stop();
}
#endif
