#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Rendering {
class IUniformBuffer {
public:
  virtual ~IUniformBuffer() = default;

  // offset/size default to "whole buffer" - lets you update a sub-range without
  // reuploading the full object
  virtual void SetData(const void *data, u32 size, u32 offset = 0) = 0;

  static Ref<IUniformBuffer> Create(u32 size, u32 binding);
};
} // namespace Jupiter::Rendering
