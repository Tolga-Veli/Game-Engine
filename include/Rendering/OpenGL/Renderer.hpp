#pragma once

#include "Rendering/Backend/IRenderer.hpp"
#include "Rendering/Backend/SceneData.hpp"

#include <glm/glm.hpp>
#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
/*
 * OpenGLRenderer: The OpenOpenGL-specific implementation of the IRenderer.
 * This class is responsible for the actual execution of GPU commands.
 */
class Renderer final : public RHI::IRenderer {
public:
  Renderer() = default;
  ~Renderer() = default;

  void Init() override {
    LOG_INFO("Renderer: Initializing Renderer");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
  }

  void SetViewport(i32 x, i32 y, i32 width, i32 height) {
    glViewport(x, y, width, height);
  }

  void Shutdown() override { LOG_INFO("Renderer: Shutdown renderer"); }

  void BeginFrame(const SceneData &sceneData) override {
    m_SceneData = sceneData;
    if (sceneData.framebufferSize.x > 0 && sceneData.framebufferSize.y > 0)
      glViewport(0, 0, sceneData.framebufferSize.x,
                 sceneData.framebufferSize.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void EndFrame() override {}

  void Submit(const RHI::RenderCommand &cmd) override;

private:
  SceneData m_SceneData{};
};
} // namespace Jupiter::Rendering::OpenGL
