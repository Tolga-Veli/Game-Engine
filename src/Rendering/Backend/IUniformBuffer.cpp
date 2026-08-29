#include "IUniformBuffer.hpp"

#include "Core/Assert.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/UniformBuffer.hpp"

namespace Jupiter::Rendering {

Ref<IUniformBuffer> IUniformBuffer::Create(u32 size, u32 binding) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_shared<OpenGL::UniformBuffer>(size, binding);
  default:
    CORE_ASSERT(false, "not implemented yet");
    return nullptr;
  }
}
} // namespace Jupiter::Rendering
