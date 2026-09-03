#include "Rendering/TextRenderer.hpp"
#include "Assets/AssetManager.hpp"
#include "Rendering/Renderer.hpp"
#include <glm/ext/matrix_clip_space.hpp>

namespace Jupiter::Rendering {
TextRenderer::TextRenderer(std::string_view path, float height, u32 maxGlyphs)
    { Init(path, height, maxGlyphs); }

void TextRenderer::Init(std::string_view path, float height, u32 maxGlyphs) {
  if (m_Initialized)
    return;

  m_MaxGlyphs = maxGlyphs;
  m_Font = Assets::AssetManager::Get<Font>(path, {.size = height});
  std::vector<u32> indices(maxGlyphs * 6);
  for (u32 i = 0; i < maxGlyphs; ++i) {
    const u32 v = i * 4, o = i * 6;
    indices[o]=v; indices[o+1]=v+1; indices[o+2]=v+2;
    indices[o+3]=v+2; indices[o+4]=v+3; indices[o+5]=v;
  }
  const RHI::BufferLayout layout{{RHI::ShaderDataType::Float2,"a_Pos",0},
                                 {RHI::ShaderDataType::Float2,"a_UV",1}};
  m_Mesh = Assets::AssetManager::Create<Mesh>("text.dynamic.mesh", [indices=std::move(indices),layout,maxGlyphs] {
    return RHI::IMesh::CreateDynamic(maxGlyphs*4*sizeof(Vertex), indices, layout);
  });
  m_Material = Assets::AssetManager::Create<Material>("text.material", [] { return RHI::IMaterial::Create(); });
  m_Atlas = Assets::AssetManager::Create<Texture2D>("text.atlas", [font=m_Font] {
    auto f=Assets::AssetManager::Resolve<Font>(font); if (!f) return Scope<RHI::ITexture>{};
    return RHI::ITexture::Create({.data=f->GetAtlasPixels(),.width=f->GetAtlasWidth(),
      .height=f->GetAtlasHeight(),.format=RHI::TextureFormat::R8});
  });
  m_Vertices.reserve(maxGlyphs*4);
  m_Initialized = true;
}
void TextRenderer::DrawText(std::string_view text, glm::vec2 pen, glm::vec3 color,
                            glm::ivec2 size) {
  if (!m_Initialized)
    return;
  auto font=Assets::AssetManager::Resolve<Font>(m_Font); auto mesh=Assets::AssetManager::Resolve<Mesh>(m_Mesh);
  auto material=Assets::AssetManager::Resolve<Material>(m_Material); auto atlas=Assets::AssetManager::Resolve<Texture2D>(m_Atlas);
  if (!font||!mesh||!material||!atlas||size.x<=0||size.y<=0) return;
  if (!m_MaterialReady) { material->SetShader(Assets::AssetManager::Get<Shader>("Shaders/text.glsl")); material->Set("u_Atlas",0); m_MaterialReady=true; }
  m_Vertices.clear();
  for (unsigned char c:text) {
    if (m_Vertices.size()/4>=m_MaxGlyphs) break;
    const auto &g=font->GetGlyph(c); const float x0=pen.x+g.bearing.x,y1=pen.y-g.bearing.y;
    const float x1=x0+g.size.x,y0=y1-g.size.y;
    m_Vertices.insert(m_Vertices.end(), {{{x0,y0},{g.uvMin.x,g.uvMax.y}},{{x1,y0},g.uvMax},{{x1,y1},{g.uvMax.x,g.uvMin.y}},{{x0,y1},g.uvMin}});
    pen.x+=g.advance;
  }
  if (m_Vertices.empty()) return;
  const glm::mat4 projection=glm::ortho(0.0f,float(size.x),0.0f,float(size.y));
  Renderer::Submit(RHI::RenderCommand{mesh,material}.UploadVertices(std::span<const Vertex>(m_Vertices))
    .SetElementCount(static_cast<u32>(m_Vertices.size()/4*6)).BindTexture(atlas).EnableAlphaBlend().DisableDepthTest()
    .ApplyOverride("u_ViewProjection",projection).ApplyOverride("u_TextColor",color));
}
}
