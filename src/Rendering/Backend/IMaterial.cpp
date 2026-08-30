#include "Rendering/Backend/IMaterial.hpp"

#include "Core/Assert.hpp"

#include "Rendering/Backend/RenderUtils.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/Material.hpp"

namespace Jupiter::Rendering::RHI {
Scope<IMaterial> IMaterial::Create() {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return CreateScope<OpenGL::Material>();
  default:
    UNREACHABLE("unsupported graphics api");
    return nullptr;
  }
}

} // namespace Jupiter::Rendering::RHI
