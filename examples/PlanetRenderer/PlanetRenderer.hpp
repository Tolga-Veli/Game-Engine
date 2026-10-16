#pragma once

#include "Assets/AssetHandle.hpp"

#include "Rendering/Mesh.hpp"
#include "Rendering/Material.hpp"

#include <glm/glm.hpp>

using namespace Jupiter;

struct PlanetRenderingInfo {
  struct PlanetRender {
    glm::vec2 pos, vel;
    float radius;
  };

  PlanetRender &operator[](std::size_t idx) noexcept { return planets[idx]; }
  const PlanetRender &operator[](std::size_t idx) const noexcept { return planets[idx]; }

  void resize(std::size_t sz) { planets.resize(sz); }
  std::size_t capacity() const noexcept { return planets.capacity(); }

  PlanetRenderingInfo() noexcept = default;
  explicit PlanetRenderingInfo(u32 cnt) noexcept : planets(cnt) {}

  std::span<const std::byte> as_span() const noexcept { return std::as_bytes(std::span{planets}); }

  std::vector<PlanetRender> planets;
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
  Assets::AssetHandle<Rendering::Shader> m_Shader;

  PlanetRenderingInfo m_Planets;

  float m_RadiusScale{2.0f}, m_MaxSpeed{50.0f}, m_GlowIntensity{1.0f};
  glm::vec3 m_ColorTint{1.0f};
};
