#include "Rendering/RendererAPI.hpp"

#include "Rendering/OpenGL/ResourceFactory.hpp"

#include <exception>
#include <cstdlib>
#include <utility>

namespace Jupiter::Rendering {

GraphicsAPI RendererAPI::s_RendererAPI = GraphicsAPI::OpenGL;
Scope<RHI::IRenderer> RendererAPI::s_RendererImplementation;
Scope<RHI::IResourceFactory> RendererAPI::s_ResourceFactory;
Scope<RHI::IRenderCommandQueue> RendererAPI::m_RenderCommandQueue;
RenderQueue RendererAPI::s_RenderQueue;

void RendererAPI::Init(GLFWwindow *window) {
  s_RenderQueue.Reset();

  auto GetFactory = []() -> Scope<RHI::IResourceFactory> {
    switch (s_RendererAPI) {
    case GraphicsAPI::OpenGL:
      return CreateScope<OpenGL::ResourceFactory>();
    default:
      std::abort();
    }
  };

  s_ResourceFactory = GetFactory();
  s_RendererImplementation->Init(window);
}

void RendererAPI::Shutdown() {
  std::exception_ptr failure;
  s_RenderQueue.Stop();

  if (s_RendererImplementation) {
    try {
      s_RendererImplementation->Shutdown();
    } catch (...) { failure = std::current_exception(); }
    s_RendererImplementation.reset();
  }

  s_ResourceFactory.reset();

  if (failure)
    std::rethrow_exception(failure);
}

bool RendererAPI::RenderNextFrame() {
  Frame *frame = s_RenderQueue.WaitForFrame();
  if (!frame)
    return false;

  s_RendererImplementation->BeginFrame(frame->sceneData);
  for (const auto &cmd : frame->commands)
    if (!cmd.mesh.IsValid() || !cmd.material.IsValid())
      s_RendererImplementation->Submit(cmd);

  for (const auto &overlay : frame->overlays)
    overlay();

  s_RendererImplementation->EndFrame();
  s_RenderQueue.ReleaseFrame(frame);
  return true;
}

void RendererAPI::RequestStop() { s_RenderQueue.Stop(); }

void RendererAPI::Wake() { s_RenderQueue.Wake(); }

void RendererAPI::BeginFrame(const SceneData &sceneData) { s_RenderQueue.BeginFrame(sceneData); }

void RendererAPI::SubmitDrawCommand(RHI::DrawCommand cmd) {
  if (!cmd.IsValid())
    return;

  s_RenderQueue.Submit(std::move(cmd));
}

void RendererAPI::SubmitOverlay(std::function<void()> overlay) {
  if (overlay)
    s_RenderQueue.SubmitOverlay(std::move(overlay));
}

void RendererAPI::EndFrame() { s_RenderQueue.EndFrame(); }

} // namespace Jupiter::Rendering
