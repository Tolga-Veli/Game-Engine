#pragma once

#include "Renderer/Backend/RenderUtils.hpp"
#include "Renderer/Backend/IRenderer.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

namespace Jupiter::Renderer {
class Renderer {
public:
  static void Init();
  static void Shutdown();
  static void OnWindowResize(u32 width, u32 height);

  static void BeginScene(const SceneData &sceneData);
  static void EndScene();

  static void Submit(const Ref<Mesh> &mesh, const Ref<Material> &material,
                     const glm::mat4 &transform = glm::mat4(1.0f));

  // Utility
  static void Clear(const glm::vec4 &color = {0.1f, 0.1f, 0.1f, 1.0f});

  inline static RendererAPI GetAPI() { return s_RendererAPI; }

private:
  static RendererAPI s_RendererAPI;
  static Scope<IRenderer> s_RendererImplementation;
};

} // namespace Jupiter::Renderer
