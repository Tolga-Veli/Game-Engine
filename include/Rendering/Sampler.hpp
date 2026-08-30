#pragma once

#include "Rendering/Backend/ISampler.hpp"

namespace Jupiter::Assets {
class AssetManager;
}

namespace Jupiter::Rendering {
class Sampler {
public:
  Sampler() = delete;
  ~Sampler() = default;

  Sampler(const Sampler &) = delete;
  Sampler &operator=(const Sampler &) = delete;
  Sampler(Sampler &&) noexcept = default;
  Sampler &operator=(Sampler &&) noexcept = default;

  [[nodiscard]]
  bool IsValid() const noexcept {
    return m_Impl != nullptr;
  }

private:
  Scope<RHI::ISampler> m_Impl;

private:
  friend class AssetManager;

  explicit Sampler(Scope<RHI::ISampler> impl) : m_Impl(std::move(impl)) {}
};
} // namespace Jupiter::Rendering
