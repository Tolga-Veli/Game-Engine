#include "Rendering/Backend/ITexture.hpp"

#include "Rendering/Backend/ITexture.hpp"
#include "Rendering/OpenGL/Texture2D.hpp"
#include "Rendering/Renderer.hpp"

#include "Core/Assert.hpp"

namespace Jupiter::Rendering::RHI {
Scope<ITexture> ITexture::Create(const RHI::TextureCreateSpec &info) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return CreateScope<OpenGL::Texture2D>(info);
  default:
    CORE_ASSERT(false, "IVertexBuffer::Create() Renderer API not implemented");
    return nullptr;
  }
}

} // namespace Jupiter::Rendering::RHI
