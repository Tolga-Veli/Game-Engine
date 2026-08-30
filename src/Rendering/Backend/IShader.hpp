#pragma once

#include "Core/Utils.hpp"

#include "Rendering/Backend/RenderUtils.hpp"
#include "Assets/AssetHandle.hpp"
#include "Rendering/Texture.hpp"

#include <string>
#include <filesystem>
#include <glm/glm.hpp>

namespace Jupiter::Rendering {

using ShaderParamValue =
    std::variant<bool, i32, u32, f32, glm::vec2, glm::vec3, glm::vec4,
                 glm::mat3, glm::mat4, Assets::AssetHandle<Texture>>;

struct ShaderParameter {
  std::string name;
  ShaderDataType type;
};

class IShader {
public:
  using ShaderSources =
      const std::array<std::string, EnumToSize(ShaderType::Count)>;
  virtual ~IShader() = default;

  static Scope<IShader> Create(const ShaderSources &shaderSrcs);

  static Scope<IShader> LoadFromFile(const std::filesystem::path &path);

  [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;

  [[nodiscard]]
  virtual const std::vector<ShaderParameter> &GetInputs() const noexcept = 0;
};
} // namespace Jupiter::Rendering
