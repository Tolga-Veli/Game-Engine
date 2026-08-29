#include "ISampler.hpp"

#include "Core/Assert.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/OpenGL/Sampler.hpp"

namespace Jupiter::Rendering {

Ref<ISampler> ISampler::Create(const SamplerSpec &spec) {
  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return std::make_shared<OpenGL::Sampler>(spec);
  default:
    CORE_ASSERT(false, "not implemented yet");
    return nullptr;
  }
}

} // namespace Jupiter::Rendering
