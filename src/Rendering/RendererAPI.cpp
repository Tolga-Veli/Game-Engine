#include "Rendering/RendererAPI.hpp"

#include "Rendering/OpenGL/ResourceFactory.hpp"
#include "Rendering/OpenGL/Renderer.hpp"

#include <exception>
#include <cstdlib>
#include <utility>

namespace Jupiter::Rendering {

std::atomic<GraphicsAPI> RendererAPI::s_RendererAPI = GraphicsAPI::OpenGL;
Scope<RHI::IRenderer> RendererAPI::s_RendererImplementation;
Scope<RHI::IResourceFactory> RendererAPI::s_ResourceFactory;
RenderScheduler RendererAPI::s_RenderScheduler;

void RendererAPI::Init(Core::Window &window) {
  s_RenderScheduler.Reset();
  s_RendererAPI.store(window.GetGraphicsAPI(), std::memory_order_release);

  switch (s_RendererAPI.load(std::memory_order_acquire)) {
  case GraphicsAPI::OpenGL:
    s_ResourceFactory = CreateScope<OpenGL::ResourceFactory>();
    s_RendererImplementation = CreateScope<OpenGL::Renderer>();
    break;
  default:
    throw std::runtime_error("Unsupported graphics API");
  }

  s_RendererImplementation->Init(window);
}

void RendererAPI::Shutdown() {
  std::exception_ptr failure;
  s_RenderScheduler.Stop();
  s_RenderScheduler.CancelPending();

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
  RenderScheduler::Work work;
  if (!s_RenderScheduler.WaitNext(work))
    return false;

  if (!work.isFrame) {
    s_RendererImplementation->InvalidateState();
    RenderExecutionContext context{*s_RendererImplementation, *s_ResourceFactory};
    work.task(context);
    return true;
  }

  try {
    s_RendererImplementation->BeginFrame(work.frame.sceneData);
    for (const auto &cmd : work.frame.commands)
      if (cmd.IsValid())
        s_RendererImplementation->Submit(cmd);

    for (const auto &overlay : work.frame.overlays) {
      s_RendererImplementation->InvalidateState();
      overlay();
    }

    s_RendererImplementation->EndFrame();
  } catch (...) {
    s_RenderScheduler.CompleteFrame();
    throw;
  }
  s_RenderScheduler.CompleteFrame(std::move(work.frame));
  return true;
}

void RendererAPI::RequestStop() { s_RenderScheduler.Stop(); }
bool RendererAPI::IsStopping() { return s_RenderScheduler.IsStopping(); }

void RendererAPI::CancelPending() { s_RenderScheduler.CancelPending(); }

bool RendererAPI::BeginFrame(const SceneData &sceneData) { return s_RenderScheduler.BeginFrame(sceneData); }

bool RendererAPI::SubmitDrawCommand(RHI::DrawCommand cmd) {
  if (!cmd.IsValid())
    return false;

  return s_RenderScheduler.Submit(std::move(cmd));
}

bool RendererAPI::SubmitOverlay(std::function<void()> overlay) {
  return s_RenderScheduler.SubmitOverlay(std::move(overlay));
}

bool RendererAPI::EndFrame() { return s_RenderScheduler.EndFrame(); }

bool RendererAPI::CommitFrame(Frame frame) { return s_RenderScheduler.CommitFrame(std::move(frame)); }

bool RendererAPI::SubmitRenderTask(RenderTask task) { return s_RenderScheduler.Enqueue(std::move(task)); }

bool RendererAPI::SetVSync(RHI::VSyncMode mode) {
  return SubmitRenderTask([mode](RenderExecutionContext &context) { context.renderer.SetVSync(mode); });
}

} // namespace Jupiter::Rendering
