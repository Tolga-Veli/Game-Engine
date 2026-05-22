#pragma once

#include "Renderer/Backend/IRenderer.hpp"

#include <glm/glm.hpp>

namespace Jupiter::Renderer {
/**
 * OpenGLRenderer: The OpenOpenGL-specific implementation of the IRenderer.
 * This class is responsible for the actual execution of GPU commands.
 */
class OpenGLRenderer : public IRenderer {
public:
  OpenGLRenderer() = default;
  virtual ~OpenGLRenderer() = default;

  virtual void Init() override;
  virtual void Shutdown() override;
  virtual void OnWindowResize(uint32_t width, uint32_t height) override;

  virtual void BeginScene(const SceneData &sceneData) override;
  virtual void EndScene() override;

  virtual void Clear(const glm::vec4 &color) override;

  virtual void Submit(const RenderCommand &cmd) override;

private:
};

} // namespace Jupiter::Renderer
