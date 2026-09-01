#pragma once

#include "Core/Utils.hpp"
#include "Core/Assert.hpp"

#include <chrono>
#include <random>
#include <glm/glm.hpp>

namespace Jupiter::Engine {
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

  void Init() {
    static std::mt19937 rng{std::random_device{}()};

    std::uniform_real_distribution<float> xDist(0.0f, m_Width),
        yDist(0.0f, m_Height);

    std::uniform_real_distribution<float> massDist(k_MinMass, k_MaxMass),
        radiusDist(k_MinRadius, k_MaxRadius);

    for (std::size_t i = 0; i < m_Planets.size(); ++i) {
      m_Planets.positions[i] = {xDist(rng), yDist(rng)};
      m_Planets.velocities[i] = {0.0f, 0.0f};
      m_Planets.masses[i] = massDist(rng);
      m_Planets.radii[i] = radiusDist(rng);
    }

    InitializeOrbitalMotion();
    CalculateAccelerations();

    m_Accumulator = 0.0f;
    m_LastStepTime = std::chrono::steady_clock::now();
  }

  void SetBodyCount(u32 count) {
    m_Planets.resize(count);
    m_Accelerations.resize(count);

    Init();
  }

  void SetBounds(float width, float height) {
    CORE_ASSERT(width > 0.0f && height > 0.0f, "width and height must be != 0");

    const float sx = width / m_Width, sy = height / m_Height;
    for (auto &pos : m_Planets.positions) {
      pos.x *= sx;
      pos.y *= sy;
    }

    m_Width = width, m_Height = height;

    for (std::size_t i = 0; i < m_Planets.size(); ++i) {
      const float radius = m_Planets.radii[i];
      auto &pos = m_Planets.positions[i];

      pos.x = glm::clamp(pos.x, radius, std::max(radius, m_Width - radius));
      pos.y = glm::clamp(pos.y, radius, std::max(radius, m_Height - radius));
    }
  }

  // call once per rendererd frame
  void Update() {
    const auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - m_LastStepTime).count();

    m_LastStepTime = now;
    dt = std::min(dt, k_MaxFrameDt);
    m_Accumulator += dt;

    u32 steps = 0;
    while (m_Accumulator >= k_FixedDt && steps < k_MaxSubsteps) {
      PhysicsStep(k_FixedDt);
      m_Accumulator -= k_FixedDt;
      ++steps;
    }
  }

  Planets &GetPlanets() { return m_Planets; }
  const Planets &GetPlanets() const { return m_Planets; }

  // Interpolation amount for rendering:
  //
  // alpha = 0:
  //    exactly the previous physics state
  //
  // alpha = 1:
  //    exactly the current physics state
  //
  // Usually render with:
  //    previous * (1-alpha) + current * alpha
  float GetInterpolationAlpha() const noexcept {
    return m_Accumulator / k_FixedDt;
  }

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

  void CalculateAccelerations() {
    const std::size_t cnt = m_Planets.size();

    const auto &positions = m_Planets.positions;
    const auto &masses = m_Planets.masses;

    std::fill(m_Accelerations.begin(), m_Accelerations.end(), glm::vec2{0.0f});

    static constexpr float softeningSq = k_Softening * k_Softening;
    for (std::size_t i = 0; i < cnt; ++i) {
      const glm::vec2 pos_i = positions[i];
      const float mass_i = masses[i];

      glm::vec2 accel_i{0.0f};

      for (std::size_t j = i + 1; j < cnt; ++j) {
        const glm::vec2 delta = positions[j] - pos_i;
        const float distSq = glm::dot(delta, delta) + softeningSq;

        const float invDist = 1.0f / std::sqrt(distSq),
                    invDist3 = invDist * invDist * invDist;

        const glm::vec2 dir = delta * invDist3;

        accel_i += dir * (k_Gravity * masses[j]);
        m_Accelerations[j] -= dir * (k_Gravity * mass_i);
      }

      m_Accelerations[i] += accel_i;
    }
  }

  void PhysicsStep(float dt) {
    const std::size_t cnt = m_Planets.size();

    auto &positions = m_Planets.positions;
    auto &velocities = m_Planets.velocities;
    const auto &radii = m_Planets.radii;

    // Velocity Verleft / Leapfrog
    // KICK + DRIFT
    // v(t + dt/2)
    // x(t + dt)

    const float halfDt = dt * 0.5f;
    for (std::size_t i{0}; i < cnt; i++) {
      glm::vec2 velocity = velocities[i], pos = positions[i];
      velocity += m_Accelerations[i] * halfDt;
      pos += velocity * dt;

      const float radius = radii[i];

      // Left
      if (pos.x - radius < 0.0f) {
        pos.x = radius;
        velocity.x = std::abs(velocity.x) * k_Bounce;
      } // Right
      else if (pos.x + radius > m_Width) {
        pos.x = m_Width - radius;
        velocity.x = -std::abs(velocity.x) * k_Bounce;
      }

      // Bottom
      if (pos.y - radius < 0.0f) {
        pos.y = radius;
        velocity.y = std::abs(velocity.y) * k_Bounce;
      } // Top
      else if (pos.y + radius > m_Height) {
        pos.y = m_Height - radius;
        velocity.y = -std::abs(velocity.y) * k_Bounce;
      }

      positions[i] = pos;
      velocities[i] = velocity;
    }

    //  Calculate acceleration at the NEW positions. / This is the only O(N²)
    //  operation.

    CalculateAccelerations();

    // FINAL KICK
    // v(t + dt)
    //
    // One final O(N) pass is required by leapfrog
    for (std::size_t i{0}; i < cnt; ++i)
      velocities[i] += m_Accelerations[i] * halfDt;

    ResolveCollisions();
  }

  void InitializeOrbitalMotion() {
    const std::size_t cnt = m_Planets.size();

    // ------------------------------------------------------------
    // Center of mass
    // ------------------------------------------------------------

    glm::vec2 center{0.0f};
    float totalMass = 0.0f;

    for (std::size_t i = 0; i < cnt; ++i) {
      center += m_Planets.positions[i] * m_Planets.masses[i];
      totalMass += m_Planets.masses[i];
    }

    if (totalMass <= 0.0f)
      return;

    center /= totalMass;

    // ------------------------------------------------------------
    // Tangential velocities
    //
    // Bodies further from the center move more slowly
    // ------------------------------------------------------------

    for (std::size_t i = 0; i < cnt; ++i) {
      const glm::vec2 offset = m_Planets.positions[i] - center;
      const float radius = glm::length(offset);

      if (radius < 1.0f)
        continue;

      // Perpendicular vector.
      const glm::vec2 tangent{-offset.y / radius, offset.x / radius};

      const float speed = std::sqrt(k_Gravity * totalMass / radius);
      m_Planets.velocities[i] = tangent * speed * k_InitialVelocityScale;
    }

    // ----------------------------------------
    // Remove center-of-mass velocity (so it doesn't drift)
    // ----------------------------------------
    glm::vec2 momentum{0.0f};
    for (std::size_t i = 0; i < cnt; ++i)
      momentum += m_Planets.velocities[i] * m_Planets.masses[i];

    const glm::vec2 centerVelocity = momentum / totalMass;
    for (auto &velocity : m_Planets.velocities)
      velocity -= centerVelocity;
  }

  void ResolveCollisions() {
    std::size_t i = 0;

    while (i < m_Planets.size()) {
      std::size_t j = i + 1;
      while (j < m_Planets.size()) {
        const glm::vec2 delta = m_Planets.positions[j] - m_Planets.positions[i];

        const float distSq = glm::dot(delta, delta),
                    radiusSum = m_Planets.radii[i] + m_Planets.radii[j];

        if (distSq < radiusSum * radiusSum) {
          MergeBodies(i, j);
          // j now contains what was at the end so test that new body against i
          continue;
        }
        ++j;
      }
      ++i;
    }
  }

  void MergeBodies(std::size_t a, std::size_t b) {
    CORE_ASSERT(a < m_Planets.size() && b < m_Planets.size(), "out of bounds");

    const float massA = m_Planets.masses[a], massB = m_Planets.masses[b],
                totalMass = massA + massB;

    m_Planets.positions[a] =
        (m_Planets.positions[a] * massA + m_Planets.positions[b] * massB) /
        totalMass;

    m_Planets.velocities[a] =
        (m_Planets.velocities[a] * massA + m_Planets.velocities[b] * massB) /
        totalMass;

    m_Planets.masses[a] = totalMass;

    // Area ∝ r², so use area-preserving
    // radius combination.
    m_Planets.radii[a] = std::sqrt(m_Planets.radii[a] * m_Planets.radii[a] +
                                   m_Planets.radii[b] * m_Planets.radii[b]);

    m_Planets.erase(b);
  }
};
} // namespace Jupiter::Engine
