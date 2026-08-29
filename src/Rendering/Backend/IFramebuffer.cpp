#include "IFramebuffer.hpp"

#include "Core/Assert.hpp"

#include "Rendering/OpenGL/Framebuffer.hpp"
#include "Rendering/Renderer.hpp"

#include <memory>

namespace Jupiter::Rendering {
Ref<IFramebuffer> IFramebuffer::Create(const FramebufferSpec &spec) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_shared<OpenGL::Framebuffer>(spec);
  default:
    CORE_ASSERT(false, "not implemented yet");
    return nullptr;
  }
}
} // namespace Jupiter::Rendering
