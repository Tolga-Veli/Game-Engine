#pragma once

#include "Core/Utils.hpp"
#include "Window.hpp"
#include "Engine/AssetManager.hpp"

#include "Renderer/Renderer.hpp"

#include <memory>
#include <vector>

namespace Jupiter {

class Application {
public:
  Application();
  ~Application();

  void Init();
  void Shutdown();

  static Scope<Application> Create() { return std::make_unique<Application>(); }

  void Run();
  void OnRender();
  void OnWindowResize(uint32_t width, uint32_t height);

  static Application &GetInstance() { return *s_Instance; }

private:
  Ref<Renderer::Mesh> m_Mesh;
  Ref<Renderer::Material> m_Material;
  glm::vec3 m_ObjectPosition = {0.0f, 0.0f, 0.0f};
  glm::mat4 m_ViewProjection;

  bool m_Running;
  static Application *s_Instance;

  Scope<Core::Window> m_Window;
  Assets::AssetManager m_AssetManager;

  // 8 Vertices (Position, Color)
  std::vector<float> cubeVertices = {
      -0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 0.0f, // 0: Front-bottom-left
      0.5f,  -0.5f, 0.5f,  0.0f, 1.0f, 0.0f, // 1: Front-bottom-right
      0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 2: Front-top-right
      -0.5f, 0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // 3: Front-top-left
      -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, // 4: Back-bottom-left
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // 5: Back-bottom-right
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 1.0f, // 6: Back-top-right
      -0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 0.0f  // 7: Back-top-left
  };

  std::vector<u32> cubeIndices = {
      0, 1, 2, 2, 3, 0, // Front
      1, 5, 6, 6, 2, 1, // Right
      7, 6, 5, 5, 4, 7, // Back
      4, 0, 3, 3, 7, 4, // Left
      4, 5, 1, 1, 0, 4, // Bottom
      3, 2, 6, 6, 7, 3  // Top
  };
};
}; // namespace Jupiter
