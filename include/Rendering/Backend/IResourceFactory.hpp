#pragma once

#include "IFont.hpp"
#include "IMaterial.hpp"
#include "IShader.hpp"
#include "IMesh.hpp"
#include "ITexture.hpp"
#include "ISampler.hpp"

namespace Jupiter::Rendering::RHI {

class IResourceFactory {
public:
  virtual ~IResourceFactory() = default;

  virtual Scope<IBuffer> CreateBuffer(const BufferDesc &desc, const BufferData &data) = 0;
  virtual Scope<IFont> CreateFont(const FontData &data) = 0;
  virtual Scope<IMaterial> CreateMaterial() = 0;
  virtual Scope<IMesh> CreateMesh(const MeshDesc &data) = 0;
  virtual Scope<IShader> CreateShader(const ShaderSources &shaderSrcs) = 0;
  virtual Scope<ITexture> CreateTexture(const TextureData &info) = 0;
  virtual Scope<ISampler> CreateSampler(const SamplerSpec &) { return {}; }
};
}; // namespace Jupiter::Rendering::RHI
