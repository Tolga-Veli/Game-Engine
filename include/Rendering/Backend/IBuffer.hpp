#pragma once

#include "Render-Utils.hpp"

namespace Jupiter::Rendering::RHI {
enum class BufferType {
  Vertex,
  Index,
  Uniform,
  Storage,
};

/*
 * Static - Upload once, use many times
 * Dynamic - Upload every frame (UI, particles)
 * Stream - Upload, use once, discard
 */
enum class BufferUsage {
  Static,
  Dynamic,
  Stream,
};

struct BufferDesc {
  BufferType type{BufferType::Vertex};
  BufferUsage usage{BufferUsage::Static};
  DataType dtype{DataType::None};
  std::size_t bytes{};
};

using BufferData = std::span<const std::byte>;

class IBuffer {
public:
  virtual ~IBuffer() = default;

  IBuffer(const IBuffer &) = delete;
  IBuffer &operator=(const IBuffer &) = delete;
  IBuffer(IBuffer &&) = delete;
  IBuffer &operator=(IBuffer &&) = delete;

  virtual void SetData(const BufferData &buffer, std::size_t offset = 0) noexcept = 0;
  // Call after GPU consumers when managing a stream buffer outside a Mesh.
  virtual void MarkUsed() noexcept {}

protected:
  IBuffer() = default;
};
} // namespace Jupiter::Rendering::RHI
