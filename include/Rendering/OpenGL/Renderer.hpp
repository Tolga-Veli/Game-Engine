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
class Renderer final : public RHI::IRenderer {
public:
  Renderer() = default;
  virtual ~Renderer() = default;

  virtual void Init() override { glEnable(GL_DEPTH_TEST); }
  virtual void Shutdown() override { delete s_SceneData; }

  virtual void BeginFrame() override {}
  virtual void EndFrame() override {}

  virtual void Submit(const RHI::RenderCommand &cmd) override;

private:
  static RHI::SceneData *s_SceneData;
};
} // namespace Jupiter::Rendering::OpenGL
