#pragma once

#include "Core/Utils.hpp"
#include <glm/glm.hpp>
#include <span>

namespace Jupiter::Rendering::RHI {
class IFont {
public:
  struct Glyph {
    glm::vec2 uvMin{}, uvMax{}, size{}, bearing{};
    float advance{};
  };

  virtual ~IFont() = default;

  static Scope<IFont> Create(const void *data, std::size_t fontSize,
                             float pixelHeight);

  virtual const Glyph &GetGlyph(unsigned char c) const = 0;

  virtual std::span<const std::byte> GetAtlasPixels() const = 0;

  virtual u32 GetAtlasWidth() const = 0;
  virtual u32 GetAtlasHeight() const = 0;
  virtual float GetLineHeight() const = 0;
};
} // namespace Jupiter::Rendering::RHI
