#pragma once

#include "Renderer/Backend/BufferLayout.hpp"
#include "RenderUtils.hpp"

#include <cstdint>
#include <glad/gl.h>

namespace Jupiter::Renderer {

class IVertexBuffer {
public:
  virtual ~IVertexBuffer() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void SetData(const void *data, u32 size) = 0;

  virtual void SetLayout(const BufferLayout &layout) = 0;
  virtual const BufferLayout &GetLayout() const = 0;

  virtual uint32_t GetVertexCount() const = 0;

  static Ref<IVertexBuffer> Create(u32 size, BufferUsage = BufferUsage::Static);
  static Ref<IVertexBuffer> Create(const void *data, u32 size,
                                   BufferUsage = BufferUsage::Static);
};

} // namespace Jupiter::Renderer
