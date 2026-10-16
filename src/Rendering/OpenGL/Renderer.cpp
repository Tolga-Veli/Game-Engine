#include "Rendering/OpenGL/Renderer.hpp"
#include "Rendering/OpenGL/Mesh.hpp"
#include "Rendering/OpenGL/Shader.hpp"
#include "Rendering/OpenGL/Texture2D.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Traits/ShaderTraits.hpp"

#include <glad/gl.h>
#include <limits>

namespace Jupiter::Rendering::OpenGL {

void Renderer::Submit(const RHI::DrawCommand &cmd) {
  if (!cmd.IsValid() || cmd.instanceCount == 0 ||
      cmd.instanceCount > static_cast<u32>(std::numeric_limits<GLsizei>::max()))
    return;

  if (m_MeshHandle != cmd.mesh) {
    m_Mesh = Assets::AssetManager::Resolve<Rendering::Mesh>(cmd.mesh);
    m_MeshHandle = cmd.mesh;
  }
  const bool materialChanged = m_MaterialHandle != cmd.material;
  if (materialChanged) {
    m_MaterialDirty = true;
    m_Material = Assets::AssetManager::Resolve<Rendering::Material>(cmd.material);
    m_MaterialHandle = cmd.material;
  }
  if (!m_Mesh || !m_Material)
    return;

  const auto shaderHandle = m_Material->GetShader();
  if (!shaderHandle.IsValid())
    return;

  if (m_ShaderHandle != shaderHandle) {
    m_MaterialDirty = true;
    m_SceneDirty = true;
    m_Shader = Assets::AssetManager::Resolve<Rendering::Shader>(shaderHandle);
    m_ShaderHandle = shaderHandle;
  }
  if (!m_Shader)
    return;

  auto &glShader = static_cast<OpenGL::Shader &>(m_Shader->GetImpl());
  auto &glMesh = static_cast<OpenGL::Mesh &>(m_Mesh->GetImpl());
  auto &vao = glMesh.GetVertexArray();
  if (!glShader.IsValid())
    return;

  if (!cmd.vertices.empty())
    if (!glMesh.UpdateVertices(cmd.vertices.as_span()))
      return;
  if (!cmd.instanceData.empty())
    if (!glMesh.UpdateInstances(cmd.instanceData.as_span()))
      return;

  if (!glMesh.GetVertexCount() || !glMesh.HasValidIndices() ||
      (glMesh.HasInstances() && cmd.instanceCount > glMesh.GetInstanceCount()))
    return;

  if (m_BlendState != cmd.alphaBlend) {
    if (cmd.alphaBlend) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else
      glDisable(GL_BLEND);
    m_BlendState = cmd.alphaBlend;
  }
  if (m_DepthState != cmd.depthTest) {
    if (cmd.depthTest)
      glEnable(GL_DEPTH_TEST);
    else
      glDisable(GL_DEPTH_TEST);
    m_DepthState = cmd.depthTest;
  }
  if (m_DepthWriteState != cmd.depthWrite) {
    glDepthMask(cmd.depthWrite ? GL_TRUE : GL_FALSE);
    m_DepthWriteState = cmd.depthWrite;
  }

  const bool shaderChanged = m_ShaderID != glShader.GetRendererID();
  if (shaderChanged) {
    glShader.Bind();
    m_ShaderID = glShader.GetRendererID();
  }
  const bool uploadMaterial = m_MaterialDirty || materialChanged || shaderChanged || cmd.texture.has_value() ||
                              m_MaterialRevision != m_Material->GetRevision();
  glShader.ResetTextureSlots(uploadMaterial ? 0 : m_MaterialTextureSlots);
  if (cmd.texture) {
    auto texture = Assets::AssetManager::Resolve<Rendering::Texture2D>(*cmd.texture);
    if (!texture)
      return;
    static_cast<OpenGL::Texture2D &>(texture->GetImpl()).Bind(0);
  }

  if ((shaderChanged || m_SceneDirty) && glShader.HasUniform("uViewProjection"))
    glShader.Set("uViewProjection", m_SceneData.viewProjection);
  m_SceneDirty = false;
  if (glShader.HasUniform("uModel"))
    glShader.Set("uModel", cmd.transform);

  if (uploadMaterial) {
    for (const auto &[name, value] : m_Material->GetProperties()) {
      glShader.Set(name, value);
      if (name == "uViewProjection")
        m_SceneDirty = true;
    }
    m_MaterialTextureSlots = glShader.GetNextTextureSlot();
    m_MaterialRevision = m_Material->GetRevision();
    m_MaterialDirty = false;
  } else {
    // The engine writes these uniforms above even when the material is clean.
    if (const auto *value = m_Material->Find("uViewProjection")) {
      glShader.Set("uViewProjection", *value);
      m_SceneDirty = true;
    }
    if (const auto *value = m_Material->Find("uModel"))
      glShader.Set("uModel", *value);
  }

  for (const auto &[name, value] : cmd.overrides) {
    glShader.Set(name, value);
    if (name == "uViewProjection")
      m_SceneDirty = true;
    if (m_Material->Has(name))
      m_MaterialDirty = true;
  }

  if (m_VertexArrayID != vao.GetRendererID()) {
    vao.Bind();
    m_VertexArrayID = vao.GetRendererID();
  }

  if (glMesh.GetIndexCount() > 0) {
    const u32 indexCount = cmd.elementCount ? cmd.elementCount : glMesh.GetIndexCount();
    if (indexCount > glMesh.GetIndexCount() || indexCount > static_cast<u32>(std::numeric_limits<GLsizei>::max()))
      return;
    if (cmd.instanceCount > 1)
      glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, cmd.instanceCount);
    else
      glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);

    glMesh.MarkUsed();
    return;
  }

  const u32 vertexCount = cmd.elementCount ? cmd.elementCount : glMesh.GetVertexCount();
  if (vertexCount > glMesh.GetVertexCount() || vertexCount > static_cast<u32>(std::numeric_limits<GLsizei>::max()))
    return;
  if (cmd.instanceCount > 1)
    glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, cmd.instanceCount);
  else
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
  glMesh.MarkUsed();
}

} // namespace Jupiter::Rendering::OpenGL
