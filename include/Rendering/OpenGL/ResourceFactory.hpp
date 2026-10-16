#pragma once

#include "Rendering/Backend/IResourceFactory.hpp"

namespace Jupiter::Rendering::OpenGL {

class ResourceFactory final : public RHI::IResourceFactory {
public:
  ~ResourceFactory() noexcept override = default;

  Scope<RHI::IBuffer> CreateBuffer(const RHI::BufferDesc &desc, const RHI::BufferData &data) override;
  Scope<RHI::IFont> CreateFont(const RHI::FontData &data) override;
  Scope<RHI::IMaterial> CreateMaterial() override;
  Scope<RHI::IMesh> CreateMesh(const RHI::MeshDesc &data) override;
  Scope<RHI::IShader> CreateShader(const RHI::ShaderSources &shaderSrcs) override;
  Scope<RHI::ITexture> CreateTexture(const RHI::TextureData &data) override;
  Scope<RHI::ISampler> CreateSampler(const RHI::SamplerSpec &spec) override;
};
}; // namespace Jupiter::Rendering::OpenGL
