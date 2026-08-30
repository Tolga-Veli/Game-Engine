#pragma once

#include "Core/Logging.hpp"

#include <chrono>

namespace Jupiter::Profiling {
class ScopedTimer {
public:
  ScopedTimer(std::string str) : label(str) {}
  ~ScopedTimer() {
    float ms = std::chrono::duration<float, std::milli>(
                   std::chrono::steady_clock::now() - start)
                   .count();
    if (ms > 2.0f)
      LOG_DEBUG("{}: {:.2f} ms", label, ms);
  }

private:
  std::string label;
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
};
} // namespace Jupiter::Profiling
