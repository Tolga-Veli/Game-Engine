#pragma once

#include "Core/Utils.hpp"
#include "Core/Logging.hpp"

#include "Rendering/Backend/ITexture.hpp"

#include <glm/glm.hpp>
#include <unordered_map>
#include <fstream>
#include <stb_truetype.h>
#include <filesystem>
#include <vector>
#include <array>

namespace Jupiter::Rendering {
class Font {
public:
  struct Glyph {
    glm::vec2 uv_min, uv_max; // atlas texture coords, 0...1
    glm::vec2 size;           // quad size in pixels at the baked size
    glm::vec2 bearing;        // offset from cursor to the glyph's top-left
    float advance;            // how far to move the cursor after this glyph
  };

  // Bakes the given .ttf's printable ASCII range (32...126)
  // into one atlas at pixelHeight and uploads it as a single-channel texture
  // atlasSize should be large enough to fit every requested glyph - 512
  // comfortably fits ASCII at most reasonable UI text sizes

  static Ref<Font> LoadFromFile(const std::filesystem::path &path,
                                float pixelHeight, u32 atlasSize = 512) {
    LOG_INFO("Reading file: {}", std::filesystem::absolute(path).string());

    std::ifstream file(path, std::ios::binary);
    if (!file) {
      LOG_ERROR("Font: Failed to open '{}'", path.string());
      return nullptr;
    }

    std::vector<unsigned char> fileData((std::istreambuf_iterator<char>(file)),
                                        std::istreambuf_iterator<char>());

    if (fileData.empty()) {
      LOG_ERROR("Font: '{}' is empty", path.string());
      return nullptr;
    }

    std::vector<unsigned char> atlasPixels(atlasSize * atlasSize);
    stbtt_pack_context packContext{};
    stbtt_PackBegin(&packContext, atlasPixels.data(),
                    static_cast<int>(atlasSize), static_cast<int>(atlasSize), 0,
                    1, nullptr);

    constexpr int firstChar = 32, charCount = 95;
    std::array<stbtt_packedchar, charCount> packedChars{};

    if (!stbtt_PackFontRange(&packContext, fileData.data(), 0, pixelHeight,
                             firstChar, charCount, packedChars.data())) {
      LOG_ERROR("Font: Failed to pack '{}'. Try a larger atlasSize",
                path.string());
      stbtt_PackEnd(&packContext);
      return nullptr;
    }

    stbtt_PackEnd(&packContext);
    auto font = std::make_shared<Font>();
    font->m_PixelHeight = pixelHeight;

    font->m_Atlas =
        ITexture::Create(atlasPixels.data(), atlasSize, atlasSize, 1);

    for (int i = 0; i < charCount; i++) {
      const auto &pc = packedChars[i];
      Glyph glyph;

      glyph.uv_min = {
          static_cast<float>(pc.x0) / static_cast<float>(atlasSize),
          static_cast<float>(pc.y0) / static_cast<float>(atlasSize),
      };

      glyph.uv_max = {
          static_cast<float>(pc.x1) / static_cast<float>(atlasSize),
          static_cast<float>(pc.y1) / static_cast<float>(atlasSize),
      };

      glyph.size = {
          static_cast<float>(pc.x1 - pc.x0),
          static_cast<float>(pc.y1 - pc.y0),
      };

      glyph.bearing = {
          pc.xoff,
          pc.yoff,
      };

      glyph.advance = pc.xadvance;

      font->m_Glyphs[static_cast<char>(firstChar + i)] = glyph;
    }

    return font;
  }

  const Glyph *GetGlyph(char c) const {
    auto it = m_Glyphs.find(c);
    if (it == m_Glyphs.end())
      return nullptr;

    return &it->second;
  }

  Ref<ITexture> GetAtlas() const noexcept { return m_Atlas; }
  float GetPixelHeight() const noexcept { return m_PixelHeight; }

private:
  Ref<ITexture> m_Atlas;
  std::unordered_map<char, Glyph> m_Glyphs;
  float m_PixelHeight{0.0f};
};
} // namespace Jupiter::Rendering
