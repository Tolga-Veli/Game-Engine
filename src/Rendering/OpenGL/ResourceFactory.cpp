#include "Core/Utils.hpp"

#include "Rendering/OpenGL/ResourceFactory.hpp"
#include "Rendering/OpenGL/Buffer.hpp"
#include "Rendering/OpenGL/Material.hpp"
#include "Rendering/OpenGL/Mesh.hpp"
#include "Rendering/OpenGL/Texture2D.hpp"
#include "Rendering/OpenGL/Shader.hpp"
#include "Rendering/OpenGL/Sampler.hpp"
#include <limits>

namespace Jupiter::Rendering::OpenGL {

Scope<RHI::IBuffer> ResourceFactory::CreateBuffer(const RHI::BufferDesc &desc, const RHI::BufferData &data) {
  return CreateScope<Buffer>(desc, data);
}

Scope<RHI::IFont> ResourceFactory::CreateFont(const RHI::FontData &data) {
  (void)data;
  return {};
}

Scope<RHI::IMaterial> ResourceFactory::CreateMaterial() { return CreateScope<Material>(); }
Scope<RHI::ISampler> ResourceFactory::CreateSampler(const RHI::SamplerSpec &spec) { return CreateScope<Sampler>(spec); }

Scope<RHI::IMesh> ResourceFactory::CreateMesh(const RHI::MeshDesc &data) {
  if (!data.layout.GetStride() || data.vertices.type != RHI::BufferType::Vertex ||
      data.indices.type != RHI::BufferType::Index || data.vertexData.size() % data.layout.GetStride() ||
      data.indexData.size() % sizeof(u32) || data.indices.bytes % sizeof(u32) ||
      data.vertexData.size() / data.layout.GetStride() > std::numeric_limits<u32>::max() ||
      data.indexData.size() / sizeof(u32) > std::numeric_limits<u32>::max())
    return {};
  auto mesh = CreateScope<Mesh>(data);
  if (!mesh->HasValidIndices())
    return {};
  return mesh;
}

Scope<RHI::IShader> ResourceFactory::CreateShader(const RHI::ShaderSources &shaderSrcs) {
  auto shader = CreateScope<Shader>(shaderSrcs);
  if (!shader->IsValid())
    return {};
  return shader;
}

Scope<RHI::ITexture> ResourceFactory::CreateTexture(const RHI::TextureData &desc) {
  if (!desc.width || !desc.height || desc.data.empty() || desc.format == RHI::TextureFormat::None)
    return {};
  const auto channels = static_cast<std::size_t>(desc.format);
  if (channels > 4 || desc.width > static_cast<u32>(std::numeric_limits<GLsizei>::max()) ||
      desc.height > static_cast<u32>(std::numeric_limits<GLsizei>::max()) ||
      desc.height > desc.data.size() / channels / desc.width)
    return {};
  GLint maxSize{};
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
  if (desc.width > static_cast<u32>(maxSize) || desc.height > static_cast<u32>(maxSize))
    return {};
  return CreateScope<Texture2D>(desc);
}

}; // namespace Jupiter::Rendering::OpenGL
