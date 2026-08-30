#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Rendering::RHI {
class ITexture {
public:
  virtual ~ITexture() = default;

  [[nodiscard]] virtual u32 GetWidth() const noexcept = 0;
  [[nodiscard]] virtual u32 GetHeight() const noexcept = 0;

  static Scope<ITexture> Create(const void *pixels, i32 width, i32 height,
                                i32 channels);

protected:
  ITexture() = default;
};

} // namespace Jupiter::Rendering::RHI
