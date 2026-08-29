#include "Framebuffer.hpp"
#include "Core/Assert.hpp"
#include "Rendering/OpenGL/VertexBuffer.hpp"

namespace Jupiter::Rendering::OpenGL {

Framebuffer::Framebuffer(const FramebufferSpec &spec) : m_Spec(spec) {
  for (auto &a : spec.attachments) {
    if (IsDepthFormat(a.format))
      m_DepthAttachmentFormat = a.format;
    else
      m_ColorAttachmentFormats.push_back(a.format);
  }
}

Framebuffer::~Framebuffer() { Cleanup(); }

void Framebuffer::Cleanup() {
  if (m_RendererID) {
    glDeleteFramebuffers(1, &m_RendererID);
    if (m_DepthAttachment) {
      glDeleteTextures(1, &m_DepthAttachment);
      m_DepthAttachment = 0;
    }
  }
  m_ColorAttachments.clear();
}

void Framebuffer::Invalidate() {
  Cleanup();
  glCreateFramebuffers(1, &m_RendererID);

  // OpenGL::Framebuffer — inside Invalidate(), replacing the raw
  // glCreateTextures loop
  m_ColorAttachments.resize(m_ColorAttachmentFormats.size());
  for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
    // Concrete type here (Ref<OpenGL::Texture>), so GetRendererID() is still
    // callable for the glNamedFramebufferTexture call below - once it's stored
    // in the member vector as Ref<ITexture> that ID is only reachable through
    // whatever accessor ITexture actually exposes (or not at all, by design).
    //  TODO: create an OpenGL::Texture constructor that acceps color attachment
    //  formats
    auto tex = Texture2D::Create(m_ColorAttachmentFormats[i], m_Spec.width,
                                 m_Spec.height);

    glNamedFramebufferTexture(m_RendererID,
                              GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                              tex->GetRendererID(), 0);
    m_ColorAttachments[i] = tex; // upcasts to Ref<ITexture> here
  }

  if (m_DepthAttachmentFormat != FramebufferTextureFormat::None) {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
    glTextureStorage2D(
        m_DepthAttachment, 1,
        JupiterToOpenGLFramebufferTextureFormat(m_DepthAttachmentFormat),
        m_Spec.width, m_Spec.height);
    glNamedFramebufferTexture(m_RendererID, GL_DEPTH_STENCIL_ATTACHMENT,
                              m_DepthAttachment, 0);
  }

  if (!m_ColorAttachments.empty()) {
    std::vector<GLenum> drawBuffers(m_ColorAttachments.size());
    for (size_t i = 0; i < drawBuffers.size(); i++)
      drawBuffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
    glNamedFramebufferDrawBuffers(m_RendererID, drawBuffers.size(),
                                  drawBuffers.data());
  } else {
    glNamedFramebufferDrawBuffer(m_RendererID,
                                 GL_NONE); // depth-only pass, e.g. shadow maps
    glNamedFramebufferReadBuffer(m_RendererID, GL_NONE);
  }

  CORE_VERIFY(glCheckNamedFramebufferStatus(m_RendererID, GL_FRAMEBUFFER) ==
                  GL_FRAMEBUFFER_COMPLETE,
              "Framebuffer: incomplete");
}

int Framebuffer::ReadPixel(u32 attachmentIndex, i32 x, i32 y) {
  CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(),
              "Framebuffer: index out of range");
  glNamedFramebufferReadBuffer(m_RendererID,
                               GL_COLOR_ATTACHMENT0 + attachmentIndex);
  int pixel;
  glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel);
  return pixel;
}

void Framebuffer::ClearAttachment(u32 attachmentIndex, i32 value) {
  CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(),
              "Framebuffer: index out of range");
  glClearTexImage(m_ColorAttachments[attachmentIndex]->GetRendererID(), 0,
                  GL_RED_INTEGER, GL_INT, &value);
}

} // namespace Jupiter::Rendering::OpenGL
