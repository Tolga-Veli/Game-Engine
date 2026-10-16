#pragma once

#include "Rendering/Backend/IMaterial.hpp"
#include "Rendering/Backend/Render-Utils.hpp"

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
    ++m_Revision;

    if (m_Impl)
      m_Impl->SetShader(m_ShaderHandle);
  }

  [[nodiscard]] Assets::AssetHandle<Shader> GetShader() const noexcept { return m_ShaderHandle; }

  [[nodiscard]] bool HasShader() const noexcept { return m_ShaderHandle.IsValid(); }

  // --------------------
  // Properties
  // --------------------
  template <class T> void Set(const std::string &name, T &&val) {
    SetParameter(name, RHI::ShaderParamValue(std::forward<T>(val)));
  }

  void SetParameter(const std::string &name, const RHI::ShaderParamValue &val) {
    auto [it, _] = m_Properties.insert_or_assign(name, val);
    ++m_Revision;

    if (m_Impl)
      m_Impl->SetProperty(name, it->second);
  }

  void Remove(const std::string &name) {
    const auto it = m_Properties.find(name);
    if (it == m_Properties.end())
      return;

    m_Properties.erase(it);
    ++m_Revision;
    if (m_Impl)
      m_Impl->RemoveProperty(name);
  }

  void Clear() {
    m_Properties.clear();
    ++m_Revision;
    if (m_Impl)
      m_Impl->ClearProperties();
  }

  [[nodiscard]] bool Has(const std::string &name) const { return m_Properties.contains(name); }

  [[nodiscard]] const RHI::ShaderParamValue *Find(const std::string &name) const {
    const auto it = m_Properties.find(name);
    if (it == m_Properties.end())
      return nullptr;

    return &it->second;
  }

  template <typename T> [[nodiscard]] const T *Get(const std::string &name) const {
    const auto *value = Find(name);
    if (!value)
      return nullptr;

    return std::get_if<T>(value);
  }

  [[nodiscard]] const std::unordered_map<std::string, RHI::ShaderParamValue> &GetProperties() const noexcept {
    return m_Properties;
  }

  [[nodiscard]] bool IsValid() const noexcept { return m_Impl != nullptr; }
  [[nodiscard]] u64 GetRevision() const noexcept { return m_Revision; }

private:
  Assets::AssetHandle<Shader> m_ShaderHandle;
  std::unordered_map<std::string, RHI::ShaderParamValue> m_Properties;
  Scope<RHI::IMaterial> m_Impl;
  u64 m_Revision{};

private:
  friend class Assets::AssetManager;

  explicit Material(Scope<RHI::IMaterial> impl) : m_Impl(std::move(impl)) {}
};
} // namespace Jupiter::Rendering
