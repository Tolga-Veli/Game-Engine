#pragma once

#include "Core/Utils.hpp"
#include "Renderer/Backend/RenderUtils.hpp"

#include <cstdint>

namespace Jupiter::Renderer {
class IIndexBuffer {
public:
  virtual ~IIndexBuffer() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual uint32_t GetCount() const = 0;

  static Ref<IIndexBuffer> Create(const u32 *indices, u32 count,
                                  BufferUsage usage = BufferUsage::Static);
};

} // namespace Jupiter::Renderer
