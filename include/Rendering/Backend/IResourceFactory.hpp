#pragma once

#include "IFont.hpp"
#include "IMaterial.hpp"
#include "IShader.hpp"
#include "IMesh.hpp"
#include "ITexture.hpp"

#include <GLFW/glfw3.h>

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
  // should i create a factory function for render command and render command queue and etc.
};
}; // namespace Jupiter::Rendering::RHI
