#pragma once

#include "Core/Utils.hpp"

#include "Rendering/Backend/RenderUtils.hpp"
#include "Assets/AssetHandle.hpp"
#include "Rendering/Texture2D.hpp"
#include "BufferLayout.hpp"

#include <string>
#include <filesystem>
#include <glm/glm.hpp>

namespace Jupiter::Rendering::RHI {

/* Shader Stages */
enum class ShaderType : u32 {
  None = 0,
  Vertex,
  Fragment,
  Geometry,
  TessControl,
  TessEval,
  Compute,

  Count
};

inline constexpr std::string_view to_string(ShaderType type) {
  using enum ShaderType;
  switch (type) {
  case None:
    return "None";
  case Vertex:
    return "Vertex";
  case Fragment:
    return "Fragment";
  case Geometry:
    return "Geometry";
  case TessControl:
    return "TessControl";
  case TessEval:
    return "TessEval";
  case Compute:
    return "Compute";
  default:
    UNREACHABLE("Undefined shader type");
    return "Unknown";
  }
}

using ShaderParamValue =
    std::variant<bool, i32, u32, f32, glm::vec2, glm::vec3, glm::vec4,
                 glm::mat3, glm::mat4, Assets::AssetHandle<Texture2D>>;

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
} // namespace Jupiter::Rendering::RHI
