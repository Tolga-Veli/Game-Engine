#include "Renderer.hpp"

#include "Rendering/Backend/IRenderer.hpp"
#include "Rendering/Backend/IShader.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Mesh.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

SceneData *Renderer::s_SceneData = nullptr;

void Renderer::Submit(const RenderCommand &cmd) {
  if (!cmd.material || !cmd.mesh)
    return;

  // Material::Bind() binds the shader and applies material uniforms
  Ref<IShader> shader = cmd.material->Bind();
  if (!shader)
    return;

  // Apply per-draw uniforms after the material has binded the shader
  for (const auto &[name, value] : cmd.overrides)
    ApplyUniform(*shader, name, value);

  auto vao = cmd.mesh->GetVertexArray();
  vao->Bind();

  const auto &ib = vao->GetIndexBuffer();

  if (ib && ib->GetCount() > 0) {
    if (cmd.instanceCount > 1)
      glDrawElementsInstanced(GL_TRIANGLES, ib->GetCount(), GL_UNSIGNED_INT,
                              nullptr, cmd.instanceCount);
    else
      glDrawElements(GL_TRIANGLES, ib->GetCount(), GL_UNSIGNED_INT, nullptr);
  } else {
    const auto &vbs = vao->GetVertexBuffers();
    if (!vbs.empty()) {
      u32 vertexCount = vbs.front()->GetVertexCount();

      if (cmd.instanceCount > 1)
        glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, cmd.instanceCount);
      else
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
  }
}

} // namespace Jupiter::Rendering::OpenGL
