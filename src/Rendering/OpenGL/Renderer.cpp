#include "Rendering/OpenGL/Renderer.hpp"
#include "Rendering/Backend/RenderCommand.hpp"
#include "Rendering/OpenGL/Mesh.hpp"
#include "Rendering/OpenGL/Shader.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Traits/ShaderTraits.hpp"
#include "Rendering/Backend/RenderCommand.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

void Renderer::Submit(const RHI::RenderCommand &cmd) {
  if (!cmd.material || !cmd.mesh)
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

  glShader.Bind();
  glShader.Set("uViewProjection", m_SceneData.viewProjection);
  glShader.Set("uModel", cmd.transform);

  for (const auto &[name, value] : cmd.material->GetProperties())
    ApplyUniform(glShader, name, value);

  for (const auto &[name, value] : cmd.overrides)
    ApplyUniform(glShader, name, value);

  vao.Bind();

  if (vao.GetIndexCount() > 0) {
    if (cmd.instanceCount > 1)
      glDrawElementsInstanced(GL_TRIANGLES, vao.GetIndexCount(),
                              GL_UNSIGNED_INT, nullptr, cmd.instanceCount);
    else
      glDrawElements(GL_TRIANGLES, vao.GetIndexCount(), GL_UNSIGNED_INT,
                     nullptr);

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
