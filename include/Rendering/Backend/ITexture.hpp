#pragma once

#include "Core/Assert.hpp"
#include "Core/Utils.hpp"

namespace Jupiter::Rendering::RHI {

enum class TextureFormat : u8 {
  None = 0,
  R8,
  RG8,
  RGB8,
  RGBA8,
};

[[nodiscard]]
inline TextureFormat ToTextureFormat(i32 channels) {
  switch (channels) {
  case 1:
    return TextureFormat::R8;
  case 2:
    return TextureFormat::RG8;
  case 3:
    return TextureFormat::RGB8;
  case 4:
    return TextureFormat::RGBA8;
  default:
    UNREACHABLE("Unsupported channel count");
    return TextureFormat::None;
  };
}

struct TextureCreateSpec {
  std::span<const std::byte> data;
  u32 width{}, height{};
  TextureFormat format{TextureFormat::RGB8};
};

class ITexture {
public:
  virtual ~ITexture() = default;

  static Scope<ITexture> Create(const TextureCreateSpec &info);

  [[nodiscard]] virtual u32 GetWidth() const noexcept = 0;
  [[nodiscard]] virtual u32 GetHeight() const noexcept = 0;
};

} // namespace Jupiter::Rendering::RHI
