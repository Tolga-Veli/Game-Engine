#pragma once

#include "Assets/AssetManager.hpp"
#include "Rendering/Backend/IMesh.hpp"
#include "Core/Utils.hpp"

namespace Jupiter::Rendering {
class Mesh {
public:
  ~Mesh() = default;

  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;
  Mesh(Mesh &&) noexcept = default;
  Mesh &operator=(Mesh &&) noexcept = default;

private:
  Scope<IMesh> m_Impl;

private:
  friend class Assets::AssetManager;

  explicit Mesh(Scope<IMesh> impl) : m_Impl(std::move(impl)) {}
};

} // namespace Jupiter::Rendering
