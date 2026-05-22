#include "Application.hpp"

#include "Renderer/Renderer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Jupiter {
Application *Application::s_Instance = nullptr;

Application::Application() { Init(); };

Application::~Application() { Shutdown(); }

void Application::Init() {
  s_Instance = this;
  m_Window = std::make_unique<Core::Window>("Jupiter Engine", 1280, 720);
  m_AssetManager.Init();
  m_Running = true;

  Renderer::Renderer::Init();

  Renderer::BufferLayout layout = {
      {Renderer::ShaderDataType::Float3, "a_Position", 0},
      {Renderer::ShaderDataType::Float3, "a_Color", 1}};

  m_Mesh = Renderer::Mesh::Create(cubeVertices, cubeIndices, layout);

  auto shader = m_AssetManager.GetShader("basic");
  m_Material = Renderer::Material::Create(shader);
  m_Material->SetVec3("u_BaseColor", {1.0f, 1.0f, 1.0f});
}

void Application::Shutdown() {
  m_Running = false;

  m_Mesh.reset();
  m_Material.reset();
  m_AssetManager.Shutdown();

  Renderer::Renderer::Shutdown();

  m_Window.reset();

  s_Instance = nullptr;
}

void Application::OnRender() {
  Jupiter::Renderer::Renderer::Clear({0.1f, 0.1f, 0.1f, 1.0f});

  Jupiter::Renderer::SceneData sceneData;
  sceneData.ViewProjectionMatrix = m_ViewProjection;

  Jupiter::Renderer::Renderer::BeginScene(sceneData);

  glm::mat4 transform =
      glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), {0.5f, 1.0f, 0.0f});

  Renderer::Renderer::Submit(m_Mesh, m_Material, transform);

  Renderer::Renderer::EndScene();
}

void Application::OnWindowResize(uint32_t width, uint32_t height) {
  Renderer::Renderer::OnWindowResize(width, height);
}

void Application::Run() {
  while (m_Running) {
    glm::mat4 proj =
        glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
    glm::mat4 view =
        glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    m_ViewProjection = proj * view;

    OnRender();

    m_Window->OnUpdate();
    if (m_Window->ShouldClose())
      m_Running = false;
  }
}

}; // namespace Jupiter
