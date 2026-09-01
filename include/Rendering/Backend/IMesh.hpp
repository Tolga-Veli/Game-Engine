#pragma once

#include "Core/Utils.hpp"
#include "Rendering/Backend/BufferLayout.hpp"

#include <span>

namespace Jupiter::Rendering::RHI {
struct MeshCreateInfo {
  std::span<const std::byte> vertexData;
  std::span<const u32> indices;
  BufferLayout layout;
};

class IMesh {
public:
  virtual ~IMesh() = default;

  static Scope<IMesh> Create(std::span<const std::byte> vertexData,
                             std::span<const u32> indices,
                             const BufferLayout &layout);

  template <typename T>
  static Scope<IMesh> Create(std::span<const T> vertices,
                             std::span<const u32> indices,
                             const BufferLayout &layout) {

    return Create(std::as_bytes(vertices), indices, layout);
  }

  [[nodiscard]]
  virtual u32 GetVertexCount() const noexcept = 0;

  [[nodiscard]]
  virtual u32 GetIndexCount() const noexcept = 0;

protected:
  IMesh() = default;
};
} // namespace Jupiter::Rendering::RHI
