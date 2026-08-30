#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Profiling {
class FpsCounter {
public:
  float Tick(float dt) {
    m_FrameCount++;
    m_Timer += dt;
    if (m_Timer >= 1.0f) {
      m_Fps = static_cast<float>(m_FrameCount) / m_Timer;
      m_FrameCount = 0;
      m_Timer = 0.0f;
    }
    return m_Fps;
  }

private:
  u32 m_FrameCount{};
  float m_Timer{}, m_Fps{};
};
} // namespace Jupiter::Profiling
