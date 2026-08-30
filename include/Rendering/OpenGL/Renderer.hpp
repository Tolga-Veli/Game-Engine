#pragma once

#include "Rendering/Backend/IRenderer.hpp"
#include "Rendering/Backend/RenderUtils.hpp"

#include <glm/glm.hpp>
#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
/*
 * OpenGLRenderer: The OpenOpenGL-specific implementation of the IRenderer.
 * This class is responsible for the actual execution of GPU commands.
 */
class Renderer final : public IRenderer {
public:
  Renderer() = default;
  virtual ~Renderer() = default;

  virtual void Init() override { glEnable(GL_DEPTH_TEST); }
  virtual void Shutdown() override { delete s_SceneData; }
  virtual void OnWindowResize(uint32_t width, uint32_t height) override {
    glViewport(0, 0, width, height);
  }

  virtual void BeginScene(const SceneData &sceneData) override {
    if (!s_SceneData)
      s_SceneData = new SceneData();
    *s_SceneData = sceneData;
  }

  virtual void EndScene() override {}

  virtual void ClearColor(const glm::vec4 &color) override {
    glClearColor(color.r, color.g, color.b, color.a);
  }

  virtual void Clear() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  virtual void SetBlendMode(BlendMode mode) override {
    using enum BlendMode;
    switch (mode) {
    case None:
      glDisable(GL_BLEND);
      break;
    case Alpha:
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
    case Additive:
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      break;
    default:
      break;
    }
  }

  virtual void SetDepthWrite(bool enabled) override {
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
  }

  virtual void Submit(const RenderCommand &cmd) override;

private:
  static SceneData *s_SceneData;
};
} // namespace Jupiter::Rendering::OpenGL
