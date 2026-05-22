#pragma once

#include "Core/Utils.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Mesh.hpp"

#include <glm/glm.hpp>

namespace Jupiter::Renderer {

struct SceneData {
  glm::mat4 ViewProjectionMatrix;
};

struct RenderCommand {
  Ref<Mesh> mesh;
  Ref<Material> material;
  glm::mat4 transform;
};

class IRenderer {
public:
  virtual ~IRenderer() = default;

  virtual void Init() = 0;
  virtual void Shutdown() = 0;
  virtual void OnWindowResize(u32 width, u32 height) = 0;

  virtual void BeginScene(const SceneData &sceneData) = 0;
  virtual void EndScene() = 0;

  virtual void Clear(const glm::vec4 &color) = 0;

  virtual void Submit(const RenderCommand &cmd) = 0;

  static Scope<IRenderer> Create();
};
}; // namespace Jupiter::Renderer
