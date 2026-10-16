#include "PlanetRenderer.hpp"

#include "Assets/AssetManager.hpp"

#include "Rendering/RendererAPI.hpp"

#include "Core/Storage.hpp"

void PlanetRenderer::Init() {
  using namespace Rendering;

  float quadVertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
  u32 quadIndices[] = {0, 1, 2, 2, 3, 0};

  Storage vertexBuffer{quadVertices}, indexBuffer{quadIndices};

  RHI::MeshDesc data;
  data.layout = {{Rendering::RHI::DataType::Float2, "a_LocalPos", 0}};

  data.vertices = RHI::BufferDesc{.type = RHI::BufferType::Vertex,
                                  .usage = RHI::BufferUsage::Static,
                                  .dtype = RHI::DataType::Float2,
                                  .bytes = vertexBuffer.size()};

  data.indices = RHI::BufferDesc{.type = RHI::BufferType::Index,
                                 .usage = RHI::BufferUsage::Static,
                                 .dtype = RHI::DataType::UInt,
                                 .bytes = indexBuffer.size()};

  data.vertexCap = 4;
  data.indexCap = 6;
  data.vertexData = std::move(vertexBuffer);
  data.indexData = std::move(indexBuffer);

  data.instanceLayout = {{RHI::DataType::Float2, "a_InstancePos", 1},
                         {RHI::DataType::Float2, "a_InstanceVelocity", 2},
                         {RHI::DataType::Float, "a_InstanceRadius", 3}};

  data.instanceCap = m_Planets.capacity();

  m_QuadMesh = Assets::AssetManager::Create<Mesh>("quad", data);

  m_Shader = Assets::AssetManager::Load<Shader>("Shaders/body_glow.glsl");
  m_Material = Assets::AssetManager::Create<Material>("body_glow", {});

  RendererAPI::SubmitRenderTask([material = m_Material, shader = m_Shader](auto &) {
    if (auto value = Assets::AssetManager::Resolve<Material>(material))
      value->SetShader(shader);
  });
}

void PlanetRenderer::SetRadiusScale(float scale) { m_RadiusScale = scale; }

void PlanetRenderer::SetMaxSpeed(float maxSpeed) { m_MaxSpeed = maxSpeed; }

void PlanetRenderer::SetGlowIntensity(float intensity) { m_GlowIntensity = intensity; }

void PlanetRenderer::SetColorTint(const glm::vec3 &tint) { m_ColorTint = tint; }

void PlanetRenderer::Draw(std::span<const glm::vec2> positions, std::span<const glm::vec2> velocities,
                          std::span<const float> radii, const glm::mat4 &viewProjection) {
  const std::size_t count = std::min({positions.size(), velocities.size(), radii.size()});
  m_Planets.resize(count);

  for (std::size_t i{0}; i < count; i++) {
    m_Planets[i].pos = positions[i];
    m_Planets[i].vel = velocities[i];
    m_Planets[i].radius = radii[i];
  }

  auto command = Rendering::RHI::DrawCommand{m_QuadMesh, m_Material}
                     .SetInstances(count)
                     .EnableAlphaBlend()
                     .DisableDepthWrite()
                     .ApplyOverride("u_ViewProjection", viewProjection)
                     .ApplyOverride("u_RadiusScale", m_RadiusScale)
                     .ApplyOverride("u_MaxSpeed", m_MaxSpeed)
                     .ApplyOverride("u_GlowIntensity", m_GlowIntensity)
                     .ApplyOverride("u_ColorTint", m_ColorTint)
                     .UploadInstances(m_Planets.as_span());

  Rendering::RendererAPI::SubmitDrawCommand(std::move(command));
}
