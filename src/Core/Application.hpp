#pragma once

#include <memory>
#include "Window.hpp"
#include "Engine/AssetManager.hpp"

#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexBuffer.hpp"

namespace Jupiter::Core {

class Application {
public:
  Application();
  virtual ~Application();

  static std::unique_ptr<Application> Create() {
    return std::make_unique<Application>();
  }

  void Run();

  static Application &GetInstance() { return *s_Instance; }

private:
  bool m_Running;
  static Application *s_Instance;

  std::unique_ptr<Window> m_Window;
  Assets::AssetManager m_AssetManager;

  std::unique_ptr<Renderer::VertexArray> m_VertexArray;
  std::unique_ptr<Renderer::VertexBuffer> m_VertexBuffer;
  std::unique_ptr<Renderer::IndexBuffer> m_IndexBuffer;
  std::shared_ptr<Renderer::Shader> m_ShaderProgram;

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

}; // namespace Jupiter::Core
