#pragma once

#include "Assets/AssetHandle.hpp"

#include "Rendering/Mesh.hpp"
#include "Rendering/Material.hpp"

#include <chrono>
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

  void SetMaxPlanets(u32 count) { m_Planets.resize(count); }

  void Init();

  void SetRadiusScale(float scale);
  void SetMaxSpeed(float maxSpeed);
  void SetGlowIntensity(float intensity);
  void SetColorTint(const glm::vec3 &tint);

  void Draw(std::span<const glm::vec2> positions,
            std::span<const glm::vec2> velocities, std::span<const float> radii,
            const glm::mat4 &viewProjection);

private:
  Assets::AssetHandle<Rendering::Mesh> m_QuadMesh;
  Assets::AssetHandle<Rendering::Material> m_Material;
  std::vector<RenderPlanetInfo> m_Planets;

  std::chrono::steady_clock::time_point m_StartTime;
  bool m_MaterialReady{};
};
