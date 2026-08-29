#pragma once

#include "Core/Assert.hpp"

#include "Rendering/Backend/IFramebuffer.hpp"
#include "Rendering/OpenGL/Texture2D.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
class Framebuffer final : public IFramebuffer {
public:
  explicit Framebuffer(const FramebufferSpec &spec);
  ~Framebuffer() override;

  Framebuffer(const Framebuffer &) = delete;
  Framebuffer &operator=(const Framebuffer &) = delete;

  void Bind() override {
    if (m_PendingResize) {
      Invalidate();
      m_PendingResize = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
    glViewport(0, 0, m_Spec.width, m_Spec.height);
  }

  void Unbind() override { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

  void Resize(i32 width, i32 height) override {
    if (width <= 0 || height <= 0)
      return;
    if (width == m_Spec.width && height == m_Spec.height)
      return;

    m_Spec.width = width;
    m_Spec.height = height;
    m_PendingResize = true; // actual creation happens once, on next Bind()
  }

  const FramebufferSpec &GetSpecification() const override { return m_Spec; }

  Ref<ITexture> GetColorAttachment(u32 index = 0) const override {
    CORE_ASSERT(index < m_ColorAttachments.size(),
                "Framebuffer: index out of range");
    return m_ColorAttachments[index];
  }

  Ref<ITexture> GetDepthAttachment() const override { return nullptr; }

  int ReadPixel(u32 attachmentIndex, i32 x, i32 y);

  void ClearAttachment(u32 attachmentIndex, i32 value);

private:
  FramebufferSpec m_Spec;
  u32 m_RendererID{};
  bool m_PendingResize{true};

  std::vector<FramebufferTextureFormat> m_ColorAttachmentFormats;
  FramebufferTextureFormat m_DepthAttachmentFormat =
      FramebufferTextureFormat::None;

  std::vector<Ref<Texture2D>> m_ColorAttachments;
  u32 m_DepthAttachment{};

  void Invalidate();
  void Cleanup();

  static bool IsDepthFormat(FramebufferTextureFormat format) {
    return format == FramebufferTextureFormat::Depth24Stencil8;
  }
};
} // namespace Jupiter::Rendering::OpenGL
