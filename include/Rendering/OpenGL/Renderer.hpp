#pragma once

#include "Rendering/Backend/IRenderer.hpp"
#include "Rendering/Backend/SceneData.hpp"
#include "Rendering/OpenGL/GraphicsContext.hpp"

#include <glm/glm.hpp>

namespace Jupiter::Rendering::OpenGL {
/*
 * OpenGLRenderer: The OpenOpenGL-specific implementation of the IRenderer.
 * This class is responsible for the actual execution of GPU commands.
 */
class Renderer final : public RHI::IRenderer {
public:
  Renderer() noexcept = default;
  ~Renderer() noexcept = default;

  void Init(GLFWwindow *window) override {
    m_GraphicsContext.SetWindowHandle(window);
    m_GraphicsContext.Init();

    LOG_INFO("Renderer: Initializing Renderer");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  void Shutdown() override {
    LOG_INFO("Renderer: Shutdown renderer");
    m_GraphicsContext.Shutdown();
  }

  void SetViewport(i32 x, i32 y, i32 width, i32 height) { glViewport(x, y, width, height); }

  void BeginFrame(const SceneData &sceneData) override {
    m_SceneData = sceneData;

    if (sceneData.framebufferSize.x > 0 && sceneData.framebufferSize.y > 0)
      glViewport(0, 0, sceneData.framebufferSize.x, sceneData.framebufferSize.y);

    glClearColor(sceneData.clearColor.r, sceneData.clearColor.g, sceneData.clearColor.b, sceneData.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void EndFrame() override { m_GraphicsContext.OnFrame(); }

  void Submit(const RHI::RenderCommand &cmd) override;

  void SetVSync(RHI::VSyncMode mode) override {
    if (mode == RHI::VSyncMode::Off)
      glfwSwapInterval(0);
    else if (mode == RHI::VSyncMode::On)
      glfwSwapInterval(1);
    else
      UNREACHABLE("No support for adaptive vsync");

    m_VSync = mode;
  }

  RHI::VSyncMode GetVSync() const noexcept { return m_VSync; }

private:
  SceneData m_SceneData{};
  GraphicsContext m_GraphicsContext;
  RHI::VSyncMode m_VSync{RHI::VSyncMode::Off};
};
} // namespace Jupiter::Rendering::OpenGL
