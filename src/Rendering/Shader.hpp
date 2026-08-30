#pragma once

#include "Core/Utils.hpp"

#include "Rendering/Backend/IShader.hpp"

#include <string_view>
#include <vector>

namespace Jupiter::Rendering {
class Shader {
public:
  ~Shader() = default;

  Shader(const Shader &) = delete;
  Shader(Shader &&) noexcept = default;
  Shader &operator=(const Shader &) = delete;
  Shader &operator=(Shader &&) noexcept = default;

  [[nodiscard]]
  std::string_view GetName() const noexcept;

  [[nodiscard]]
  const std::vector<ShaderParameter> &GetInputs() const noexcept;

  [[nodiscard]]
  const Ref<IShader> &GetImplementation() const noexcept {
    return m_Impl;
  }

private:
  Ref<IShader> m_Impl;

  explicit Shader(Ref<IShader> impl) : m_Impl(std::move(impl)) {}
};

} // namespace Jupiter::Rendering
