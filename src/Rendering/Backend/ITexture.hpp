#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Rendering {
class ITexture {
public:
  virtual ~ITexture() = default;

  ITexture(const ITexture &) = delete;
  ITexture(ITexture &&) = delete;
  ITexture &operator=(const ITexture &) = delete;
  ITexture &operator=(ITexture &&) = delete;

  [[nodiscard]] virtual u32 GetWidth() const noexcept = 0;
  [[nodiscard]] virtual u32 GetHeight() const noexcept = 0;

  virtual void Bind(u32 slot = 0) const noexcept = 0;
  virtual void Unbind() const noexcept = 0;

  static Scope<ITexture> Create(const void *pixels, i32 width, i32 height,
                                i32 channels);

protected:
  ITexture() = default;
};

} // namespace Jupiter::Rendering
