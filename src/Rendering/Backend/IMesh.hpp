#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Rendering {
class IMesh {
public:
  virtual ~IMesh() = default;

  static Scope<IMesh> Create(const void *vertices, u32 vertexCount,
                             u32 vertexStride, const void *indices,
                             u32 indexCount);

private:
};
} // namespace Jupiter::Rendering
