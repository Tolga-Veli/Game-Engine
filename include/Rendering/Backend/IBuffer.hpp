#pragma once

#include "Core/Utils.hpp"

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
  BufferType type;
  BufferUsage usage;
  std::size_t size;
};

class IBuffer {
public:
  virtual ~IBuffer() = default;

  IBuffer(const IBuffer &) = delete;
  IBuffer &operator=(const IBuffer &) = delete;
  IBuffer(IBuffer &&) = delete;
  IBuffer &operator=(IBuffer &&) = delete;

  [[nodiscard]]
  virtual u32 GetSize() const noexcept = 0;

  virtual void SetData(const void *data, u32 size, u32 offset = 0) noexcept = 0;

protected:
  IBuffer() = default;
};
} // namespace Jupiter::Rendering::RHI
