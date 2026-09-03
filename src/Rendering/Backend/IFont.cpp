#include "Rendering/Backend/IFont.hpp"
#include <array>
#include <stb_truetype.h>
#include <vector>

namespace Jupiter::Rendering::RHI {
namespace {
class StbFont final : public IFont {
public:
  StbFont(const void *data, std::size_t size, float height)
      : m_Pixels(kAtlasSize * kAtlasSize), m_LineHeight(height) {
    stbtt_fontinfo info{};
    const auto *bytes = static_cast<const unsigned char *>(data);
    if (!data || size == 0 || !stbtt_InitFont(&info, bytes, 0)) return;
    std::array<stbtt_packedchar, 95> packed{};
    stbtt_pack_context context{};
    if (!stbtt_PackBegin(&context, m_Pixels.data(), kAtlasSize, kAtlasSize, 0, 1, nullptr)) return;
    stbtt_PackSetOversampling(&context, 2, 2);
    const bool ok = stbtt_PackFontRange(&context, bytes, 0, height, 32, 95, packed.data()) != 0;
    stbtt_PackEnd(&context);
    if (!ok) return;
    for (std::size_t i = 0; i < packed.size(); ++i) {
      const auto &p = packed[i]; auto &g = m_Glyphs[i];
      g.uvMin = {float(p.x0) / kAtlasSize, float(p.y0) / kAtlasSize};
      g.uvMax = {float(p.x1) / kAtlasSize, float(p.y1) / kAtlasSize};
      // Packed rectangles include oversampling pixels. xoff2/yoff2 are the
      // actual display-space bounds and therefore match xadvance.
      g.size = {p.xoff2 - p.xoff, p.yoff2 - p.yoff};
      g.bearing = {p.xoff, p.yoff}; g.advance = p.xadvance;
    }
    m_Valid = true;
  }
  bool IsValid() const { return m_Valid; }
  const Glyph &GetGlyph(unsigned char c) const override { static const Glyph empty{}; return c >= 32 && c < 127 ? m_Glyphs[c - 32] : empty; }
  std::span<const std::byte> GetAtlasPixels() const override { return {reinterpret_cast<const std::byte *>(m_Pixels.data()), m_Pixels.size()}; }
  u32 GetAtlasWidth() const override { return kAtlasSize; }
  u32 GetAtlasHeight() const override { return kAtlasSize; }
  float GetLineHeight() const override { return m_LineHeight; }
private:
  static constexpr u32 kAtlasSize = 512;
  std::vector<unsigned char> m_Pixels; std::array<Glyph, 95> m_Glyphs{};
  float m_LineHeight{}; bool m_Valid{};
};
}
Scope<IFont> IFont::Create(const void *data, std::size_t fontSize, float pixelHeight) {
  auto font = CreateScope<StbFont>(data, fontSize, pixelHeight);
  return font->IsValid() ? std::move(font) : nullptr;
}
}
