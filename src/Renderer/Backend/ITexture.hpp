#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Renderer {
class ITexture {
public:
  virtual ~ITexture() = default;

  virtual u32 GetWidth() const = 0;
  virtual u32 GetHeight() const = 0;
  virtual u32 GetRendererID() const = 0;

  virtual void Bind(u32 slot = 0) const = 0;
  virtual void Unbind() const = 0;

  static Ref<ITexture> Create(const std::string &path);
};

} // namespace Jupiter::Renderer
