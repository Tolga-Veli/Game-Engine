#include "PlanetRenderer.hpp"

#include "Assets/AssetManager.hpp"

#include "Rendering/Renderer.hpp"

void PlanetRenderer::Init() {
  constexpr float quadVertices[] = {-0.5f, -0.5f, 0.5f,  -0.5f,
                                    0.5f,  0.5f,  -0.5f, 0.5f};

  constexpr u32 quadIndices[] = {0, 1, 2, 2, 3, 0};

  Rendering::RHI::BufferLayout quadLayout = {
      {Rendering::RHI::ShaderDataType::Float2, "a_LocalPos", 0}};

  m_QuadMesh = Assets::AssetManager::Create<Rendering::Mesh>(
      "quad", [quadVertices, quadIndices, quadLayout]() {
        return Rendering::RHI::IMesh::Create(
            std::span<const float>(quadVertices),
            std::span<const u32>(quadIndices), quadLayout);
      });

  m_Material = Assets::AssetManager::Create<Rendering::Material>(
      "body_glow", []() { return Rendering::RHI::IMaterial::Create(); });
}

void PlanetRenderer::SetRadiusScale(float scale) {
  m_RadiusScale = scale;
}

void PlanetRenderer::SetMaxSpeed(float maxSpeed) {
  m_MaxSpeed = maxSpeed;
}

void PlanetRenderer::SetGlowIntensity(float intensity) {
  m_GlowIntensity = intensity;
}

void PlanetRenderer::SetColorTint(const glm::vec3 &tint) {
  m_ColorTint = tint;
}

void PlanetRenderer::Draw(std::span<const glm::vec2> positions,
                          std::span<const glm::vec2> velocities,
                          std::span<const float> radii,
                          const glm::mat4 &viewProjection) {
  auto mesh = Assets::AssetManager::Resolve<Rendering::Mesh>(m_QuadMesh);
  auto material =
      Assets::AssetManager::Resolve<Rendering::Material>(m_Material);

  if (!mesh || !material)
    return;

  if (!m_MaterialReady) {
    material->SetShader(
        Assets::AssetManager::Get<Rendering::Shader>("Shaders/body_glow.glsl"));
    m_MaterialReady = true;
  }

  const std::size_t count =
      std::min({positions.size(), velocities.size(), radii.size()});
  m_Planets.resize(count);

  for (std::size_t i{0}; i < count; i++) {
    m_Planets[i].pos = positions[i];
    m_Planets[i].vel = velocities[i];
    m_Planets[i].radius = radii[i];
  }

  auto command = Rendering::RHI::RenderCommand{mesh, material}
                     .SetInstances(static_cast<u32>(count))
                     .EnableAlphaBlend()
                     .ApplyOverride("u_ViewProjection", viewProjection)
                     .ApplyOverride("u_RadiusScale", m_RadiusScale)
                     .ApplyOverride("u_MaxSpeed", m_MaxSpeed)
                     .ApplyOverride("u_GlowIntensity", m_GlowIntensity)
                     .ApplyOverride("u_ColorTint", m_ColorTint);
  command.UploadInstances(
      std::span<const RenderPlanetInfo>(m_Planets.data(), count));
  Rendering::Renderer::Submit(std::move(command));
}
