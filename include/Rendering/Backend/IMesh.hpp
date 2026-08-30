#pragma once

#include "Core/Utils.hpp"

#include "Rendering/Backend/BufferLayout.hpp"

namespace Jupiter::Rendering::RHI {
class IMesh {
public:
  virtual ~IMesh() = default;

  static Scope<IMesh> Create(const std::vector<float> &vertices,
                             const std::vector<u32> &indices,
                             const BufferLayout &layout);
};
} // namespace Jupiter::Rendering::RHI
