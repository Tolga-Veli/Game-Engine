#pragma once

#include "Rendering/Backend/IVertexArray.hpp"

#include "Assets/Types.hpp"

namespace Jupiter::Rendering {
class Mesh {
public:
  Mesh() = default;
  Mesh(const Ref<IVertexArray> &vertexArray) : m_VertexArray(vertexArray) {}

  virtual ~Mesh() = default;

  static Assets::AssetHandle<Mesh> Create(std::string_view,
                                          std::vector<float> vertices,
                                          std::vector<u32> indices,
                                          BufferLayout layout);

  void Bind() const { m_VertexArray->Bind(); }

  Ref<IVertexArray> GetVertexArray() const { return m_VertexArray; }

private:
  Ref<IVertexArray> m_VertexArray;
};

} // namespace Jupiter::Rendering
