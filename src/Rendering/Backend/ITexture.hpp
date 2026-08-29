#pragma once

#include "Core/Utils.hpp"

#include <filesystem>

namespace Jupiter::Rendering {
class ITexture {
public:
  virtual ~ITexture() = default;

  virtual u32 GetWidth() const = 0;
  virtual u32 GetHeight() const = 0;

  virtual void Bind(u32 slot = 0) const = 0;
  virtual void Unbind() const = 0;

  static Ref<ITexture> LoadFromFile(const std::filesystem::path &path);
  static Ref<ITexture> Create(const void *pixels, u32 width, u32 height,
                              u32 channels = 4);
};

} // namespace Jupiter::Rendering
