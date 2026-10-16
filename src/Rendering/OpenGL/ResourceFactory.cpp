#include "Core/Utils.hpp"

#include "Rendering/OpenGL/ResourceFactory.hpp"
#include "Rendering/OpenGL/Renderer.hpp"
#include "Rendering/OpenGL/Buffer.hpp"
#include "Rendering/OpenGL/Material.hpp"
#include "Rendering/OpenGL/Mesh.hpp"
#include "Rendering/OpenGL/Shader.hpp"
#include "Rendering/OpenGL/Texture2D.hpp"
#include "Rendering/OpenGL/Shader.hpp"

namespace Jupiter::Rendering::OpenGL {

Scope<RHI::IBuffer> ResourceFactory::CreateBuffer(const RHI::BufferDesc &desc, const RHI::BufferData &data) {
  return CreateScope<Buffer>(desc);
}

Scope<RHI::IFont> ResourceFactory::CreateFont(const RHI::FontData &data) {
  (void)data;
  return {};
}

Scope<RHI::IMaterial> ResourceFactory::CreateMaterial() { return CreateScope<Material>(); }

Scope<RHI::IMesh> ResourceFactory::CreateMesh(const RHI::MeshDesc &data) { return CreateScope<Mesh>(data); }

Scope<RHI::IShader> ResourceFactory::CreateShader(const RHI::ShaderSources &shaderSrcs) {
  return CreateScope<Shader>(shaderSrcs);
}

Scope<RHI::ITexture> ResourceFactory::CreateTexture(const RHI::TextureData &desc) {
  return CreateScope<Texture2D>(desc);
}

}; // namespace Jupiter::Rendering::OpenGL
