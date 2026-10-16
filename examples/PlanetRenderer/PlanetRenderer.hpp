#pragma once

#include "Assets/AssetHandle.hpp"

#include "Rendering/Mesh.hpp"
#include "Rendering/Material.hpp"

#include <glm/glm.hpp>

using namespace Jupiter;

struct RenderPlanetInfo {
  glm::vec2 pos, vel;
  float radius;
};

class PlanetRenderer {
public:
  PlanetRenderer() = default;
  explicit PlanetRenderer(u32 maxPlanets) : m_Planets(maxPlanets) { Init(); }

  void Init();

  void SetRadiusScale(float scale);
  void SetMaxSpeed(float maxSpeed);
  void SetGlowIntensity(float intensity);
  void SetColorTint(const glm::vec3 &tint);

  void Draw(std::span<const glm::vec2> positions, std::span<const glm::vec2> velocities, std::span<const float> radii,
            const glm::mat4 &viewProjection);

private:
  Assets::AssetHandle<Rendering::Mesh> m_QuadMesh;
  Assets::AssetHandle<Rendering::Material> m_Material;
  std::vector<RenderPlanetInfo> m_Planets;

  float m_RadiusScale{2.0f}, m_MaxSpeed{50.0f}, m_GlowIntensity{1.0f};
  glm::vec3 m_ColorTint{1.0f};
  bool m_MaterialReady{};
};
