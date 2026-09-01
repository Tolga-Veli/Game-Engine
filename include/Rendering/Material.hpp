#pragma once

#include "Rendering/Backend/IMaterial.hpp"
#include "Rendering/Backend/ShaderTypes.hpp"

#include <unordered_map>

namespace Jupiter::Assets {
class AssetManager;
}

namespace Jupiter::Rendering {
class Material {
public:
  Material() = delete;
  ~Material() noexcept = default;

  Material(const Material &) = delete;
  Material &operator=(const Material &) = delete;
  Material(Material &&) noexcept = default;
  Material &operator=(Material &&) noexcept = default;

  void SetShader(Assets::AssetHandle<Shader> shader) {
    CORE_ASSERT(shader.IsValid(), "Invalid shader handle");
    CORE_ASSERT(m_Impl, "m_Impl is a nullptr");
    m_ShaderHandle = shader;

    if (m_Impl)
      m_Impl->SetShader(m_ShaderHandle);
  }

  [[nodiscard]]
  Assets::AssetHandle<Shader> GetShader() const noexcept {
    return m_ShaderHandle;
  }

  [[nodiscard]]
  bool HasShader() const noexcept {
    return m_ShaderHandle.IsValid();
  }

  // --------------------
  // Properties
  // --------------------
  template <class T> void Set(std::string_view name, T &&val) {
    SetParameter(name, RHI::ShaderParamValue(std::forward<T>(val)));
  }

  void SetParameter(std::string_view name, const RHI::ShaderParamValue &val) {
    auto [it, _] = m_Properties.insert_or_assign(std::string(name), val);

    if (m_Impl)
      m_Impl->SetProperty(name, it->second);
  }

  void Remove(std::string_view name) {
    const auto it = m_Properties.find(std::string(name));
    if (it == m_Properties.end())
      return;

    m_Properties.erase(it);

    if (m_Impl)
      m_Impl->RemoveProperty(name);
  }

  void Clear() {
    m_Properties.clear();

    if (m_Impl)
      m_Impl->ClearProperties();
  }

  [[nodiscard]]
  bool Has(std::string_view name) const {
    return m_Properties.contains(std::string(name));
  }

  [[nodiscard]]
  const RHI::ShaderParamValue *Find(std::string_view name) const {
    const auto it = m_Properties.find(std::string(name));

    if (it == m_Properties.end())
      return nullptr;

    return &it->second;
  }

  template <typename T>
  [[nodiscard]]
  const T *Get(std::string_view name) const {
    const auto *value = Find(name);

    if (!value)
      return nullptr;

    return std::get_if<T>(value);
  }

  [[nodiscard]]
  const std::unordered_map<std::string, RHI::ShaderParamValue> &
  GetProperties() const noexcept {
    return m_Properties;
  }

  [[nodiscard]]
  bool IsValid() const noexcept {
    return m_Impl != nullptr;
  }

private:
  Assets::AssetHandle<Shader> m_ShaderHandle;
  std::unordered_map<std::string, RHI::ShaderParamValue> m_Properties;
  Scope<RHI::IMaterial> m_Impl;

private:
  friend class Assets::AssetManager;

  explicit Material(Scope<RHI::IMaterial> impl) : m_Impl(std::move(impl)) {}
};
} // namespace Jupiter::Rendering
