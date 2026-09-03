#pragma once
#include "Assets/AssetHandle.hpp"
#include "Rendering/Font.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Texture2D.hpp"
#include <glm/glm.hpp>
#include <string_view>

namespace Jupiter::Rendering {
class TextRenderer {
public:
  TextRenderer() = default;
  explicit TextRenderer(std::string_view fontPath, float pixelHeight = 24.0f,
                        u32 maxGlyphs = 256);

  void Init(std::string_view fontPath = "Fonts/Montserrat-Regular.ttf",
            float pixelHeight = 24.0f, u32 maxGlyphs = 256);

  void DrawText(std::string_view text, glm::vec2 position, glm::vec3 color,
                glm::ivec2 framebufferSize);

private:
  struct Vertex {
    glm::vec2 position, uv;
  };

  Assets::AssetHandle<Font> m_Font;
  Assets::AssetHandle<Mesh> m_Mesh;
  Assets::AssetHandle<Material> m_Material;
  Assets::AssetHandle<Texture2D> m_Atlas;

  std::vector<Vertex> m_Vertices;
  u32 m_MaxGlyphs{};
  bool m_MaterialReady{};
  bool m_Initialized{};
};

} // namespace Jupiter::Rendering
