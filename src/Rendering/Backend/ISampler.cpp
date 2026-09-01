#include "Rendering/Backend/ISampler.hpp"

#include "Core/Assert.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/Sampler.hpp"

namespace Jupiter::Rendering::RHI {

Scope<ISampler> ISampler::Create(const SamplerSpec &spec) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return CreateScope<OpenGL::Sampler>(spec);
  default:
    UNREACHABLE("Unsupported graphics api");
    return {};
  }
}

} // namespace Jupiter::Rendering::RHI
