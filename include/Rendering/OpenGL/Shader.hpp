#pragma once

#include "Rendering/Backend/IShader.hpp"

#include <unordered_map>
#include <variant>

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {

class Shader final : public RHI::IShader {
public:
  explicit Shader(const ShaderSources &shaderSrcs);

  ~Shader() override;

  [[nodiscard]]
  std::string_view GetName() const noexcept override {
    return m_Name;
  }

  [[nodiscard]]
  const std::vector<RHI::ShaderParameter> &GetInputs() const noexcept override {
    return m_Inputs;
  }

  void Set(std::string_view name, const RHI::ShaderParamValue &value) {

    std::visit([this, name](const auto &value) { Set(name, value); }, value);
  }

  void Set(std::string_view name, bool value) override;
  void Set(std::string_view name, i32 value) override;
  void Set(std::string_view name, u32 value) override;
  void Set(std::string_view name, f32 value) override;

  void Set(std::string_view name, const glm::vec2 &value) override;
  void Set(std::string_view name, const glm::vec3 &value) override;
  void Set(std::string_view name, const glm::vec4 &value) override;

  void Set(std::string_view name, const glm::mat3 &value) override;
  void Set(std::string_view name, const glm::mat4 &value) override;
  void Set(std::string_view name, const RHI::TextureBinding &binding) override;

  void ResetTextureSlots() noexcept { m_NextTextureSlot = 0; }

private:
  u32 m_RendererID{};
  std::string m_Name;

  std::vector<RHI::ShaderParameter> m_Inputs;

  std::unordered_map<std::string, i32> m_UniformLocCache;

  u32 m_NextTextureSlot{};

  void Compile(RHI::ShaderType type, std::string_view source);
  void Reflect();

  [[nodiscard]]
  i32 GetUniformLoc(std::string_view name);

  [[nodiscard]]
  u32 AllocateTextureSlot() noexcept {
    return m_NextTextureSlot++;
  }
};

} // namespace Jupiter::Rendering::OpenGL
