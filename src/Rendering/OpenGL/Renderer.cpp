#include "Rendering/OpenGL/Renderer.hpp"
#include "Rendering/Backend/RenderCommand.hpp"
#include "Rendering/OpenGL/Mesh.hpp"
#include "Rendering/OpenGL/Shader.hpp"
#include "Rendering/OpenGL/Texture2D.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Traits/ShaderTraits.hpp"
#include "Rendering/Backend/RenderCommand.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

void Renderer::Submit(const RHI::RenderCommand &cmd) {
  if (!cmd.material || !cmd.mesh || cmd.instanceCount == 0)
    return;

  const auto shaderHandle = cmd.material->GetShader();
  if (!shaderHandle.IsValid())
    return;

  auto shader = Assets::AssetManager::Resolve<Rendering::Shader>(shaderHandle);
  if (!shader)
    return;

  auto &glShader = static_cast<OpenGL::Shader &>(shader->GetImpl());
  auto &glMesh = static_cast<OpenGL::Mesh &>(cmd.mesh->GetImpl());
  auto &vao = glMesh.GetVertexArray();

  if (!cmd.vertices.empty())
    glMesh.SetVertexData(StorageView{const_cast<Storage &>(cmd.vertices)});
  if (!cmd.instanceData.empty())
    glMesh.SetInstanceData(StorageView{const_cast<Storage &>(cmd.instanceData)});

  if (cmd.alphaBlend) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else
    glDisable(GL_BLEND);

  if (cmd.depthTest)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);

  glShader.Bind();
  if (cmd.texture)
    static_cast<OpenGL::Texture2D &>(cmd.texture->GetImpl()).Bind(0);

  if (glShader.HasUniform("uViewProjection"))
    glShader.Set("uViewProjection", m_SceneData.viewProjection);
  if (glShader.HasUniform("uModel"))
    glShader.Set("uModel", cmd.transform);

  for (const auto &[name, value] : cmd.material->GetProperties())
    ApplyUniform(glShader, name, value);

  for (const auto &[name, value] : cmd.overrides)
    ApplyUniform(glShader, name, value);

  vao.Bind();

  if (vao.GetIndexCount() > 0) {
    const u32 indexCount = cmd.elementCount ? cmd.elementCount : vao.GetIndexCount();
    if (cmd.instanceCount > 1)
      glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, cmd.instanceCount);
    else
      glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);

    return;
  }

  const auto &vertexCounts = vao.GetVertexCounts();
  if (vertexCounts.empty())
    return;

  const u32 vertexCount = vertexCounts.front();
  if (cmd.instanceCount > 1)
    glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, cmd.instanceCount);
  else
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

} // namespace Jupiter::Rendering::OpenGL
