#pragma once

#include "Rendering/Backend/IShader.hpp"

#include <string_view>
#include <vector>

namespace Jupiter::Assets {
class AssetManager;
}

namespace Jupiter::Rendering {
class Shader {
public:
  Shader() = delete;
  ~Shader() = default;

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;
  Shader(Shader &&) noexcept = default;
  Shader &operator=(Shader &&) noexcept = default;

  [[nodiscard]]
  std::string_view GetName() const noexcept {
    return m_Impl->GetName();
  }

  [[nodiscard]]
  const std::vector<RHI::ShaderParameter> &GetInputs() const noexcept {
    return m_Impl->GetInputs();
  }

  [[nodiscard]]
  RHI::IShader &GetImpl() noexcept {
    return *m_Impl;
  }

  [[nodiscard]]
  const RHI::IShader &GetImpl() const noexcept {
    return *m_Impl;
  }

private:
  Scope<RHI::IShader> m_Impl;

private:
  friend class Assets::AssetManager;

  explicit Shader(Scope<RHI::IShader> impl) : m_Impl(std::move(impl)) {}
};

} // namespace Jupiter::Rendering
