#pragma once

#include "Core/Utils.hpp"
#include "Core/Assert.hpp"

#include <chrono>
#include <glm/glm.hpp>

using namespace Jupiter;

struct Planets {
  Planets() = default;

  explicit Planets(u32 cnt)
      : positions(cnt), velocities(cnt), masses(cnt), radii(cnt) {}

  void resize(u32 cnt) {
    positions.resize(cnt);
    velocities.resize(cnt);
    masses.resize(cnt);
    radii.resize(cnt);
  }

  void reserve(u32 cnt) {
    positions.reserve(cnt);
    velocities.reserve(cnt);
    masses.reserve(cnt);
    radii.reserve(cnt);
  }

  void erase(std::size_t idx) {
    CORE_ASSERT(idx < positions.size(), "out of bounds");

    const std::size_t last = positions.size() - 1;
    if (idx != last) {
      positions[idx] = positions[last];
      velocities[idx] = velocities[last];
      masses[idx] = masses[last];
      radii[idx] = radii[last];
    }

    positions.pop_back();
    velocities.pop_back();
    masses.pop_back();
    radii.pop_back();
  }

  std::size_t size() const noexcept { return positions.size(); }

  std::vector<glm::vec2> positions, velocities;
  std::vector<float> masses, radii;
};

class PlanetSimulation {
public:
  PlanetSimulation() = default;
  PlanetSimulation(u32 planetCount, float width = 1280.0f,
                   float height = 720.0f)
      : m_Planets(planetCount), m_Accelerations(planetCount), m_Width(width),
        m_Height(height) {
    Init();
  }

  void Init();

  void SetBodyCount(u32 count) {
    m_Planets.resize(count);
    m_Accelerations.resize(count);

    Init();
  }

  void SetBounds(float width, float height);

  void Update();

  Planets &GetPlanets() { return m_Planets; }
  const Planets &GetPlanets() const { return m_Planets; }

private:
  // k_Gravity - intentionally NOT real-world G.
  //
  // k_Softening - Plummer softening, Larger = smoother close encounters
  //
  // k_FixedDt - 120 Hz physics
  //
  // k_MaxFrameDt - Maximum dt for one frame
  //
  // k_MaxSubsteps - prevent spiral-of-death
  //
  // k_InitialVelocityScale - < 1 keeps the initial system more thightly bound
  static constexpr float k_Gravity = 6.6743e-1, k_Softening = 4.0f,
                         k_Bounce = 0.9f, k_FixedDt = 1.0f / 120.0f,
                         k_MaxFrameDt = 0.1f, k_MinMass = 100.0f,
                         k_MaxMass = 10000.0f, k_MinRadius = 2.0f,
                         k_MaxRadius = 4.0f, k_InitialVelocityScale = 1.0f;

  static constexpr std::size_t k_MaxSubsteps = 8;

  Planets m_Planets;
  std::vector<glm::vec2> m_Accelerations;
  float m_Width{}, m_Height{}, m_Accumulator{};

  std::chrono::steady_clock::time_point m_LastStepTime;

  void CalculateAccelerations();
  void PhysicsStep(float dt);
  void InitializeOrbitalMotion();
  void ResolveCollisions();
  void MergeBodies(std::size_t a, std::size_t b);
};
