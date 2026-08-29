#include "ITexture.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/Texture2D.hpp"

#include "Core/Logging.hpp"
#include "Core/Assert.hpp"

#include <stb_image.h>

namespace Jupiter::Rendering {
Ref<ITexture> ITexture::Create(const void *pixels, u32 width, u32 height,
                               u32 channels) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return OpenGL::Texture2D::Create(pixels, width, height, channels);
  default:
    CORE_ASSERT(false, "IVertexBuffer::Create() Renderer API not implemented");
    return nullptr;
  }
}

Ref<ITexture> ITexture::LoadFromFile(const std::filesystem::path &path) {
  int width, height, channels;
  stbi_set_flip_vertically_on_load(true);
  stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

  if (!data) {
    LOG_ERROR("Failed to load texture: {}", path.string());
    return nullptr;
  }

  auto texture = Create(data, width, height, channels);
  stbi_image_free(data);
  return texture;
}

} // namespace Jupiter::Rendering
