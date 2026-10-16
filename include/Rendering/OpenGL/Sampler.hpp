#pragma once
#include "Core/Assert.hpp"
#include "Rendering/Backend/ISampler.hpp"
#include "Rendering/OpenGL/OpenGL-Utils.hpp"
#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

class Sampler final : public RHI::ISampler {
public:
  explicit Sampler(const RHI::SamplerSpec &spec) noexcept;
  ~Sampler() noexcept override { glDeleteSamplers(1, &m_RendererID); }

  Sampler(const Sampler &) = delete;
  Sampler &operator=(const Sampler &) = delete;
  Sampler(Sampler &&) noexcept;
  Sampler &operator=(Sampler &&) noexcept;

  [[nodiscard]] u32 GetRendererID() const noexcept { return m_RendererID; }

  void Bind(u32 slot) const noexcept { glBindSampler(slot, m_RendererID); }

private:
  u32 m_RendererID{};
};
} // namespace Jupiter::Rendering::OpenGL
