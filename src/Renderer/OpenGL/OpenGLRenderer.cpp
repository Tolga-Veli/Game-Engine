#include "OpenGLRenderer.hpp"

#include <glad/gl.h>

namespace Jupiter::Renderer {

static SceneData *s_SceneData = nullptr;

void OpenGLRenderer::Init() { glEnable(GL_DEPTH_TEST); }

void OpenGLRenderer::Shutdown() { delete s_SceneData; }

void OpenGLRenderer::OnWindowResize(uint32_t width, uint32_t height) {
  glViewport(0, 0, width, height);
}

void OpenGLRenderer::Clear(const glm::vec4 &color) {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::BeginScene(const SceneData &sceneData) {
  if (!s_SceneData)
    s_SceneData = new SceneData();
  *s_SceneData = sceneData;
}

void OpenGLRenderer::EndScene() {}

void OpenGLRenderer::Submit(const RenderCommand &cmd) {
  auto shader = cmd.material->GetShader();
  shader->Bind();

  shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
  shader->SetMat4("u_Transform", cmd.transform);

  cmd.material->ApplyParameters();

  auto vao = cmd.mesh->GetVertexArray();
  vao->Bind();

  const auto &ib = vao->GetIndexBuffer();
  if (ib && ib->GetCount() > 0)
    glDrawElements(GL_TRIANGLES, ib->GetCount(), GL_UNSIGNED_INT, nullptr);
  else {
    const auto &vbs = vao->GetVertexBuffers();
    if (!vbs.empty())
      glDrawArrays(GL_TRIANGLES, 0, vbs.front()->GetVertexCount());
  }
}

} // namespace Jupiter::Renderer
