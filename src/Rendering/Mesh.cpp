#include "Mesh.hpp"
#include "Assets/AssetManager.hpp"
#include "Rendering/Backend/IIndexBuffer.hpp"
#include "Rendering/Backend/IVertexArray.hpp"
#include "Rendering/Backend/IVertexBuffer.hpp"

namespace Jupiter::Rendering {
Assets::AssetHandle<Mesh> Mesh::Create(std::string_view name,
                                       std::vector<float> vertices,
                                       std::vector<u32> indices,
                                       BufferLayout layout) {
  return Assets::AssetManager::Create<Mesh>(
      name,
      [vertices = std::move(vertices), indices = std::move(indices),
       layout = std::move(layout)]() mutable -> Ref<Mesh> {
        auto vao = IVertexArray::Create();

        auto vbo = IVertexBuffer::Create(vertices.data(),
                                         vertices.size() * sizeof(float));

        vbo->SetLayout(layout);
        vao->AddVertexBuffer(vbo);

        auto ibo = IIndexBuffer::Create(indices.data(), indices.size());
        vao->SetIndexBuffer(ibo);
        return std::make_shared<Mesh>(vao);
      });
}

} // namespace Jupiter::Rendering
