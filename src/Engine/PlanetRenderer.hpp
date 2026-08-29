#pragma once

#include "Assets/AssetManager.hpp"

#include "Rendering/Backend/IVertexBuffer.hpp"
#include "Rendering/Backend/RenderUtils.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Renderer.hpp"

#include <chrono>
#include <glm/glm.hpp>

namespace Jupiter::Rendering {

struct BodyGPU {
  glm::vec2 pos, vel;
  float radius;
};

class PlanetRenderer {
public:
  PlanetRenderer() = default;
  explicit PlanetRenderer(u32 maxBodies) : m_InstanceData(maxBodies) { Init(); }

  void SetMaxBodies(u32 maxBodies) { m_InstanceData.resize(maxBodies); }

  void Init() {
    std::vector<float> quadVertices = {-0.5f, -0.5f, 0.5f,  -0.5f,
                                       0.5f,  0.5f,  -0.5f, 0.5f};

    std::vector<u32> quadIndices = {0, 1, 2, 2, 3, 0};

    BufferLayout quadLayout = {{ShaderDataType::Float2, "a_LocalPos", 0}};

    m_QuadMesh = Mesh::Create(quadVertices, quadIndices, quadLayout);

    m_InstanceBuffer = IVertexBuffer::Create(
        m_InstanceData.size() * sizeof(BodyGPU), BufferUsage::Dynamic);

    m_InstanceBuffer->SetLayout(
        {{ShaderDataType::Float2, "a_InstancePos", 1},
         {ShaderDataType::Float2, "a_InstanceVelocity", 2},
         {ShaderDataType::Float, "a_InstanceRadius", 3}});
    m_QuadMesh->GetVertexArray()->AddVertexBuffer(m_InstanceBuffer, 1);

    m_Material = Material::Create(
        Assets::AssetManager::Get<IShader>("Shaders/body_glow.glsl"));

    m_Material->SetFloat("u_RadiusScale", 1.5f);
    m_Material->SetFloat("u_MaxSpeed", 15.0f);
    m_Material->SetFloat("u_GlowIntensity", 1.0f);
    m_Material->SetVec3("u_ColorTint", glm::vec3(0.7f, 0.2f, 1.0f));

    m_StartTime = std::chrono::steady_clock::now();
  }

  void SetRadiusScale(float scale) {
    m_Material->SetFloat("u_RadiusScale", scale);
  }

  void SetMaxSpeed(float maxSpeed) {
    m_Material->SetFloat("u_MaxSpeed", maxSpeed);
  }

  void SetGlowIntensity(float intensity) {
    m_Material->SetFloat("u_GlowIntensity", intensity);
  }

  // Multiplies the speed-gradient color -- {1,1,1} leaves it untouched,
  // anything else tints the whole palette warmer/cooler.
  void SetColorTint(const glm::vec3 &tint) {
    m_Material->SetVec3("u_ColorTint", tint);
  }

  void Draw(std::span<const glm::vec2> positions,
            std::span<const glm::vec2> velocities, std::span<const float> radii,
            const glm::mat4 &viewProjection) {
    u32 count = std::min({positions.size(), velocities.size(), radii.size(),
                          m_InstanceData.size()});
    if (count == 0)
      return;

    for (std::size_t i{0}; i < count; i++) {
      m_InstanceData[i].pos = positions[i];
      m_InstanceData[i].vel = velocities[i];
      m_InstanceData[i].radius = radii[i];
    }

    m_InstanceBuffer->SetData(m_InstanceData.data(), count * sizeof(BodyGPU));

    const float elapsed = std::chrono::duration<float>(
                              std::chrono::steady_clock::now() - m_StartTime)
                              .count();

    Renderer::SetBlendMode(BlendMode::Alpha);

    Renderer::Submit(RenderCommand{m_QuadMesh, m_Material}
                         .SetInstances(count)
                         .ApplyOverride("u_ViewProjection", viewProjection)
                         .ApplyOverride("u_Time", elapsed));
  }

private:
  Ref<Mesh> m_QuadMesh;
  Ref<IVertexBuffer> m_InstanceBuffer;
  Ref<Material> m_Material;

  std::vector<BodyGPU> m_InstanceData;

  std::chrono::steady_clock::time_point m_StartTime;
};
} // namespace Jupiter::Rendering
