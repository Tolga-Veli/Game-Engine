#pragma once

#include "Core/Utils.hpp"
#include "Core/Storage.hpp"

#include <glm/glm.hpp>

namespace Jupiter::Rendering::RHI {

struct FontOptions {
  float size = 16.0f;
};

struct FontData {
  Storage data;
  FontOptions opts;
};

class IFont {
public:
  struct Glyph {};

  virtual ~IFont() = default;

  virtual const Glyph &GetGlyph(char c) const = 0;

  virtual std::span<const std::byte> GetAtlasPixels() const = 0;

  virtual u32 GetAtlasWidth() const = 0;
  virtual u32 GetAtlasHeight() const = 0;
  virtual float GetLineHeight() const = 0;
};
} // namespace Jupiter::Rendering::RHI
