#include "ITexture.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/Texture2D.hpp"

#include "Core/Assert.hpp"

namespace Jupiter::Rendering {
Scope<ITexture> ITexture::Create(const void *pixels, i32 width, i32 height,
                                 i32 channels) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return CreateScope<OpenGL::Texture2D>(pixels, width, height, channels);
  default:
    CORE_ASSERT(false, "IVertexBuffer::Create() Renderer API not implemented");
    return nullptr;
  }
}

} // namespace Jupiter::Rendering
