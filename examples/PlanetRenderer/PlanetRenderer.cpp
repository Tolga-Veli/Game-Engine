#include "PlanetRenderer.hpp"

#include "Assets/AssetManager.hpp"

#include "Rendering/RendererAPI.hpp"

#include "Core/Storage.hpp"

void PlanetRenderer::Init() {
  float quadVertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
  u32 quadIndices[] = {0, 1, 2, 2, 3, 0};

  Storage vertexBuffer{quadVertices}, indexBuffer{quadIndices};

  Rendering::RHI::BufferData vertices{
      .type = Rendering::RHI::BufferType::Vertex,
      .usage = Rendering::RHI::BufferUsage::Static,
      .data = std::move(vertexBuffer),
  };

  Rendering::RHI::BufferData indices{.type = Rendering::RHI::BufferType::Index,
                                     .usage = Rendering::RHI::BufferUsage::Static,
                                     .data = std::move(indexBuffer)};

  Rendering::RHI::BufferLayout quadLayout = {{Rendering::RHI::ShaderDataType::Float2, "a_LocalPos", 0}};

  Rendering::RHI::MeshData data{
      .vertices = std::move(vertices), .indices = std::move(indices), .layout = std::move(quadLayout)};

  m_QuadMesh = Assets::AssetManager::Create<Rendering::Mesh>("quad", data);

  m_Material = Assets::AssetManager::Create<Rendering::Material>("body_glow", {});
}

void PlanetRenderer::SetRadiusScale(float scale) { m_RadiusScale = scale; }

void PlanetRenderer::SetMaxSpeed(float maxSpeed) { m_MaxSpeed = maxSpeed; }

void PlanetRenderer::SetGlowIntensity(float intensity) { m_GlowIntensity = intensity; }

void PlanetRenderer::SetColorTint(const glm::vec3 &tint) { m_ColorTint = tint; }

void PlanetRenderer::Draw(std::span<const glm::vec2> positions, std::span<const glm::vec2> velocities,
                          std::span<const float> radii, const glm::mat4 &viewProjection) {
  auto mesh = Assets::AssetManager::Resolve<Rendering::Mesh>(m_QuadMesh);
  auto material = Assets::AssetManager::Resolve<Rendering::Material>(m_Material);

  if (!mesh || !material)
    return;

  if (!m_MaterialReady) {
    material->SetShader(Assets::AssetManager::Load<Rendering::Shader>("Shaders/body_glow.glsl"));
    m_MaterialReady = true;
  }

  const std::size_t count = std::min({positions.size(), velocities.size(), radii.size()});
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
                     .ApplyOverride("u_ColorTint", m_ColorTint)
                     .UploadInstances(Storage{m_Planets});

  Rendering::RendererAPI::Submit(std::move(command));
}
