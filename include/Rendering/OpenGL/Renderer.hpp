#pragma once

#include "Rendering/Backend/IRenderer.hpp"
#include "Rendering/Backend/SceneData.hpp"
#include "Rendering/OpenGL/GraphicsContext.hpp"
#include "Core/Window.hpp"

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

  void Init(Core::Window &window) override {
    m_GraphicsContext = CreateScope<GraphicsContext>(window.GetNativeHandle());
    m_GraphicsContext->Init();

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
    ResetResources();
    if (m_GraphicsContext) {
      m_GraphicsContext->Shutdown();
      m_GraphicsContext.reset();
    }
  }

  void SetViewport(i32 x, i32 y, i32 width, i32 height) { glViewport(x, y, width, height); }

  void BeginFrame(const SceneData &sceneData) override {
    m_SceneData = sceneData;
    // Tasks and overlays may change GL state between frames.
    InvalidateState();
    m_DepthWriteState = 1;
    glDepthMask(GL_TRUE);

    if (sceneData.framebufferSize.x > 0 && sceneData.framebufferSize.y > 0)
      glViewport(0, 0, sceneData.framebufferSize.x, sceneData.framebufferSize.y);

    glClearColor(sceneData.clearColor.r, sceneData.clearColor.g, sceneData.clearColor.b, sceneData.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void EndFrame() override {
    ResetResources();
    m_GraphicsContext->OnFrame();
  }

  void Submit(const RHI::DrawCommand &cmd) override;

  void InvalidateState() override {
    ResetResources();
    m_BlendState = m_DepthState = m_DepthWriteState = -1;
    m_ShaderID = m_VertexArrayID = 0;
    m_SceneDirty = true;
    m_MaterialDirty = true;
  }

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
  Scope<GraphicsContext> m_GraphicsContext;
  RHI::VSyncMode m_VSync{RHI::VSyncMode::Off};

  i32 m_BlendState{-1}, m_DepthState{-1}, m_DepthWriteState{-1};
  u32 m_ShaderID{}, m_VertexArrayID{}, m_MaterialTextureSlots{};
  u64 m_MaterialRevision{};
  bool m_SceneDirty{true}, m_MaterialDirty{true};

  Assets::AssetHandle<Rendering::Mesh> m_MeshHandle;
  Assets::AssetHandle<Rendering::Material> m_MaterialHandle;
  Assets::AssetHandle<Rendering::Shader> m_ShaderHandle;

  Ref<Rendering::Mesh> m_Mesh;
  Ref<Rendering::Material> m_Material;
  Ref<Rendering::Shader> m_Shader;

  void ResetResources() noexcept {
    m_Mesh.reset();
    m_Material.reset();
    m_Shader.reset();
    m_MeshHandle = {};
    m_MaterialHandle = {};
    m_ShaderHandle = {};
  }
};
} // namespace Jupiter::Rendering::OpenGL
