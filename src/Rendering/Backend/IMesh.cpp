#include "Rendering/Backend/IMesh.hpp"

#include "Core/Assert.hpp"
#include "Rendering/OpenGL/Mesh.hpp"
#include "Rendering/Renderer.hpp"

namespace Jupiter::Rendering::RHI {

Scope<IMesh> IMesh::Create(std::span<const std::byte> vertexData,
                           std::span<const u32> indices,
                           const BufferLayout &layout) {

  MeshCreateInfo info{
      .vertexData = vertexData,
      .indices = indices,
      .layout = layout,
  };

  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return CreateScope<OpenGL::Mesh>(info);
  default:
    UNREACHABLE("Unsupported graphics api");
    return {};
  }
}

Scope<IMesh> IMesh::CreateDynamic(std::size_t vertexCapacity,
                                  std::span<const u32> indices,
                                  const BufferLayout &layout) {
  CORE_ASSERT(vertexCapacity > 0, "Dynamic mesh capacity cannot be zero");

  MeshCreateInfo info{.vertexData = {},
                      .indices = indices,
                      .layout = layout,
                      .usage = BufferUsage::Dynamic,
                      .vertexCapacity = vertexCapacity};

  switch (Renderer::GetAPI()) {
  case GraphicsAPI::OpenGL:
    return CreateScope<OpenGL::Mesh>(info);
  default:
    UNREACHABLE("Unsupported graphics api");
    return {};
  }
}

} // namespace Jupiter::Rendering::RHI
