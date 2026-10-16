#pragma once

#include "Rendering/Backend/IMesh.hpp"
#include "Core/Utils.hpp"

namespace Jupiter::Assets {
class AssetManager;
}

namespace Jupiter::Rendering {

class Mesh {
public:
  Mesh() = delete;
  ~Mesh() noexcept = default;

  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;
  Mesh(Mesh &&) noexcept = default;
  Mesh &operator=(Mesh &&) noexcept = default;

  [[nodiscard]] u32 GetVertexCount() const noexcept { return m_Impl->GetVertexCount(); }
  [[nodiscard]] u32 GetVertexCapacity() const noexcept { return m_Impl->GetVertexCapacity(); }
  [[nodiscard]] u32 GetIndexCount() const noexcept { return m_Impl->GetIndexCount(); }
  [[nodiscard]] u32 GetIndexCapacity() const noexcept { return m_Impl->GetIndexCapacity(); }
  [[nodiscard]] RHI::IMesh &GetImpl() noexcept { return *m_Impl; }

private:
  Scope<RHI::IMesh> m_Impl;

private:
  friend class Assets::AssetManager;

  explicit Mesh(Scope<RHI::IMesh> impl) : m_Impl(std::move(impl)) {}
};

} // namespace Jupiter::Rendering
