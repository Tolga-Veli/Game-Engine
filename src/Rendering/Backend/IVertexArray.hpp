#pragma once

#include "IVertexBuffer.hpp"
#include "IIndexBuffer.hpp"

namespace Jupiter::Rendering {
class IVertexArray {
public:
  virtual ~IVertexArray() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  // divisor = 0 - advance this buffer's attributes per vertex (the default) use
  // this for actual mesh geometry
  // divisor = 1 - advance per instance use this for a buffer olding one value
  // (a position, a color) per instance of an instanced draw
  virtual void AddVertexBuffer(const Ref<IVertexBuffer> &vertexBuffer,
                               u32 divisor = 0) = 0;

  virtual void SetIndexBuffer(const Ref<IIndexBuffer> &indexBuffer) = 0;

  virtual const std::vector<Ref<IVertexBuffer>> &GetVertexBuffers() const = 0;

  virtual const Ref<IIndexBuffer> &GetIndexBuffer() const = 0;

  static Ref<IVertexArray> Create();
};
} // namespace Jupiter::Rendering
