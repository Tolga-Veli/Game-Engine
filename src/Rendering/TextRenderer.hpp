#pragma once

#include "Core/Assert.hpp"

#include "Assets/AssetManager.hpp"

#include "Renderer.hpp"

#include "Rendering/Backend/IIndexBuffer.hpp"
#include "Rendering/Backend/IVertexArray.hpp"
#include "Rendering/Backend/IVertexBuffer.hpp"

#include "Mesh.hpp"
#include "Material.hpp"
#include "Font.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Jupiter::Rendering {

// Rebuilds a dynamic quad-per-glyph buffer on every DrawText call. Fine
// for UI/HUD text -- a handful of short strings a frame;
class TextRenderer {
public:
  TextRenderer() noexcept = default;
  explicit TextRenderer(u32 maxCharacters) : m_MaxCharacters(maxCharacters) {
    Init();
  }

  void SetMaxCharacters(u32 characters) { m_MaxCharacters = characters; }

  void Init() {
    CORE_ASSERT(m_MaxCharacters > 0,
                "TextRenderer max character count is invalid");

    BufferLayout layout = {
        {ShaderDataType::Float2, "a_Pos", 0},
        {ShaderDataType::Float2, "a_UV", 1},
    };

    m_Vertices.resize(m_MaxCharacters * 4);
    m_VertexBuffer = IVertexBuffer::Create(
        static_cast<u32>(m_Vertices.size() * sizeof(Vertex)),
        BufferUsage::Dynamic);

    m_VertexBuffer->SetLayout(layout);

    auto vao = IVertexArray::Create();
    vao->AddVertexBuffer(m_VertexBuffer);

    std::vector<u32> indices;
    indices.reserve(m_MaxCharacters * 6);

    for (u32 i = 0; i < m_MaxCharacters; i++) {
      u32 base = i * 4;
      indices.push_back(base);
      indices.push_back(base + 1);
      indices.push_back(base + 2);

      indices.push_back(base + 2);
      indices.push_back(base + 3);
      indices.push_back(base);
    }

    auto ibo =
        IIndexBuffer::Create(indices.data(), static_cast<u32>(indices.size()));

    vao->SetIndexBuffer(ibo);

    m_Mesh = std::make_shared<Mesh>(vao);
    m_Material = Material::Create(
        Assets::AssetManager::Get<IShader>("Shaders/text.glsl"));
  }

  // screenProjection: an orthographic matrix in screen/pixel space,
  // independent of the world camera -- so panning/zooming the sim
  // doesn't move on-screen labels
  void DrawText(const Ref<Font> &font, std::string_view text,
                glm::vec2 position, const glm::vec3 &color,
                const glm::mat4 &screenProjection) {
    if (!font || text.empty())
      return;

    u32 quadCount = 0;
    auto cursor = position;

    for (char c : text) {
      if (quadCount >= m_MaxCharacters)
        break;

      if (c == '\n') {
        cursor.x = position.x;
        cursor.y += font->GetPixelHeight();
        continue;
      }

      const auto *glyph = font->GetGlyph(c);

      if (!glyph)
        continue;

      /*
       * position/cursor represents the baseline.

       * With our screen-space projection:
       *
       *   X -> right
       *   Y -> down
       *
       * stbtt's packed glyph offset therefore maps directly into
       * this coordinate system.
       */
      const auto topLeft = cursor + glyph->bearing;
      const auto bottomRight = topLeft + glyph->size;

      Vertex *quad = &m_Vertices[quadCount * 4];

      quad[0] = {
          topLeft,
          {glyph->uv_min.x, glyph->uv_min.y},
      };

      quad[1] = {
          {bottomRight.x, topLeft.y},
          {glyph->uv_max.x, glyph->uv_min.y},
      };

      quad[2] = {
          bottomRight,
          {glyph->uv_max.x, glyph->uv_max.y},
      };

      quad[3] = {
          {topLeft.x, bottomRight.y},
          {glyph->uv_min.x, glyph->uv_max.y},
      };

      cursor.x += glyph->advance;
      ++quadCount;
    }

    if (quadCount == 0)
      return;

    // Only upload the vertices that are actually being used.
    m_VertexBuffer->SetData(m_Vertices.data(), quadCount * 4 * sizeof(Vertex));
    m_Material->SetTexture("u_Atlas", font->GetAtlas());
    m_Material->SetVec3("u_TextColor", color);

    Renderer::SetBlendMode(BlendMode::Alpha);
    Renderer::Submit(RenderCommand{m_Mesh, m_Material}.ApplyOverride(
        "u_ViewProjection", screenProjection));
  }

private:
  struct Vertex {
    glm::vec2 Position, UV;
  };

  u32 m_MaxCharacters{0};

  Ref<Mesh> m_Mesh;
  Ref<IVertexBuffer> m_VertexBuffer;
  Ref<Material> m_Material;

  std::vector<Vertex> m_Vertices;
};

} // namespace Jupiter::Rendering
