#pragma once

#include "Core/Utils.hpp"
#include "Rendering/Backend/ITexture.hpp"

#include <vector>

namespace Jupiter::Rendering::RHI {
enum class FramebufferTextureFormat : u8 {
  None = 0,
  RGBA8,
  RGBA16F,
  RedInteger, // e.g. entity-ID picking
  Depth24Stencil8,
};

struct FramebufferAttachmentSpec {
  FramebufferTextureFormat format;
};

struct FramebufferSpec {
  i32 width = 0, height = 0, samples = 1;
  std::vector<FramebufferAttachmentSpec> attachments;
};

class IFramebuffer {
public:
  virtual ~IFramebuffer() = default;

  static Ref<IFramebuffer> Create(const FramebufferSpec &spec);

  virtual void Bind() = 0;
  virtual void Unbind() = 0;
  virtual void Resize(i32 width, i32 height) = 0;

  virtual const FramebufferSpec &GetSpecification() const = 0;

  virtual Ref<ITexture> GetColorAttachment(u32 idx = 0) const = 0;
  virtual Ref<ITexture> GetDepthAttachment() const = 0;
};
} // namespace Jupiter::Rendering::RHI
