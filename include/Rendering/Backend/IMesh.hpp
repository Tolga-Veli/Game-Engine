#pragma once

#include "Core/Utils.hpp"
#include "Rendering/Backend/BufferLayout.hpp"
#include "Rendering/Backend/IBuffer.hpp"

#include <span>

namespace Jupiter::Rendering::RHI {
struct MeshCreateInfo {
  std::span<const std::byte> vertexData;
  std::span<const u32> indices;
  BufferLayout layout;
  BufferUsage usage{BufferUsage::Static};
  std::size_t vertexCapacity{};
};

class IMesh {
public:
  virtual ~IMesh() = default;

  static Scope<IMesh> Create(std::span<const std::byte> vertexData,
                             std::span<const u32> indices,
                             const BufferLayout &layout);

  static Scope<IMesh> CreateDynamic(std::size_t vertexCapacity,
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

  virtual void SetVertexData(std::span<const std::byte> data) = 0;

protected:
  IMesh() = default;
};
} // namespace Jupiter::Rendering::RHI
