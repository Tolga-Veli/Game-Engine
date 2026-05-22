#pragma once

#include "Renderer/Backend/IVertexArray.hpp"
#include "Renderer/Backend/IVertexBuffer.hpp"
#include "Renderer/Backend/IIndexBuffer.hpp"

namespace Jupiter::Renderer {

class Mesh {
public:
  Mesh(const Ref<IVertexArray> &vertexArray) : m_VertexArray(vertexArray) {}

  virtual ~Mesh() = default;

  static Ref<Mesh> Create(const std::vector<float> &vertices,
                          const std::vector<u32> &indices,
                          const BufferLayout &layout) {
    auto vao = IVertexArray::Create();
    auto vbo = Ref<IVertexBuffer>(IVertexBuffer::Create(
        vertices.data(), vertices.size() * sizeof(float)));

    vbo->SetLayout(layout);
    vao->AddVertexBuffer(vbo);

    auto ibo = IIndexBuffer::Create(indices.data(), indices.size());
    vao->SetIndexBuffer(ibo);
    return std::make_shared<Mesh>(vao);
  }

  void Bind() const { m_VertexArray->Bind(); }
  Ref<IVertexArray> GetVertexArray() const { return m_VertexArray; }

private:
  Ref<IVertexArray> m_VertexArray;
};

} // namespace Jupiter::Renderer
