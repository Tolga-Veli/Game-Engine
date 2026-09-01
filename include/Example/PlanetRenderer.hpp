#pragma once

#include "Assets/AssetManager.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/Backend/RenderCommand.hpp"

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
    constexpr float quadVertices[] = {-0.5f, -0.5f, 0.5f,  -0.5f,
                                      0.5f,  0.5f,  -0.5f, 0.5f};

    constexpr u32 quadIndices[] = {0, 1, 2, 2, 3, 0};

    RHI::BufferLayout quadLayout = {
        {RHI::ShaderDataType::Float2, "a_LocalPos", 0}};

    m_QuadMesh = Assets::AssetManager::Create<Mesh>(
        "quad", [quadVertices, quadIndices, quadLayout]() {
          return RHI::IMesh::Create(std::span<const float>(quadVertices),
                                    std::span<const u32>(quadIndices),
                                    quadLayout);
        });

    m_Material = Assets::AssetManager::Create<Material>(
        "body_glow", []() { return RHI::IMaterial::Create(); });

    auto material = Assets::AssetManager::Resolve<Material>(m_Material);
    material->SetShader(
        Assets::AssetManager::Get<Shader>("Shaders/body_glow.glsl"));

    material->Set("u_RadiusScale", 1.5f);
    material->Set("u_MaxSpeed", 15.0f);
    material->Set("u_GlowIntensity", 1.0f);
    material->Set("u_ColorTint", glm::vec3{0.7f, 0.2f, 1.0f});

    m_StartTime = std::chrono::steady_clock::now();
  }

  void SetRadiusScale(float scale) {
    auto material = Assets::AssetManager::Resolve<Material>(m_Material);
    if (material)
      material->Set("u_RadiusScale", scale);
  }

  void SetMaxSpeed(float maxSpeed) {
    auto material = Assets::AssetManager::Resolve<Material>(m_Material);
    if (material)
      material->Set("u_MaxSpeed", maxSpeed);
  }

  void SetGlowIntensity(float intensity) {
    auto material = Assets::AssetManager::Resolve<Material>(m_Material);
    if (material)
      material->Set("u_GlowIntensity", intensity);
  }

  void SetColorTint(const glm::vec3 &tint) {
    auto material = Assets::AssetManager::Resolve<Material>(m_Material);
    if (material)
      material->Set("u_ColorTint", tint);
  }

  void Draw(std::span<const glm::vec2> positions,
            std::span<const glm::vec2> velocities, std::span<const float> radii,
            const glm::mat4 &viewProjection) {
    const u32 count = std::min({positions.size(), velocities.size(),
                                radii.size(), m_InstanceData.size()});
    for (std::size_t i{0}; i < count; i++) {
      m_InstanceData[i].pos = positions[i];
      m_InstanceData[i].vel = velocities[i];
      m_InstanceData[i].radius = radii[i];
    }

    /* * TODO: * * Upload m_InstanceData to the RHI instance buffer. * * The
     * instance buffer should be owned by the mesh/RHI mesh and * configured
     * with: * * binding = 1 * divisor = 1 * * Do not access OpenGL::VertexArray
     * here. */

    const float elapsed = std::chrono::duration<float>(
                              std::chrono::steady_clock::now() - m_StartTime)
                              .count();

    // TODO:
    // Renderer::SetBlendMode(BlendMode::Alpha);

    Renderer::Submit(
        RHI::RenderCommand{Assets::AssetManager::Resolve<Mesh>(m_QuadMesh),
                           Assets::AssetManager::Resolve<Material>(m_Material)}
            .SetInstances(count)
            .ApplyOverride("u_ViewProjection", viewProjection)
            .ApplyOverride("u_Time", elapsed));
  }

private:
  Assets::AssetHandle<Mesh> m_QuadMesh;
  Assets::AssetHandle<Material> m_Material;

  std::vector<BodyGPU> m_InstanceData;

  std::chrono::steady_clock::time_point m_StartTime;
};
} // namespace Jupiter::Rendering
