#pragma once

#include "Core/Utils.hpp"
#include "Core/Assert.hpp"
#include "Assets/AssetHandle.hpp"
#include "Rendering/Texture2D.hpp"
#include "Rendering/Sampler.hpp"

#include <variant>

namespace Jupiter::Rendering::RHI {
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
enum class ShaderDataType : u32 {
  None = 0,

  Bool,

  Int,
  Int2,
  Int3,
  Int4,

  UInt,
  UInt2,
  UInt3,
  UInt4,

  Float,
  Float2,
  Float3,
  Float4,

  Mat3,
  Mat4,

  Texture2D,

  Count
};

[[nodiscard]]
constexpr u32 ShaderDataTypeSize(ShaderDataType type) {
  using enum ShaderDataType;
  switch (type) {
  case Bool:
    return sizeof(bool);

  case Int:
    return sizeof(int);
  case Int2:
    return sizeof(int) * 2;
  case Int3:
    return sizeof(int) * 3;
  case Int4:
    return sizeof(int) * 4;

  case UInt:
    return sizeof(unsigned int);
  case UInt2:
    return sizeof(unsigned int) * 2;
  case UInt3:
    return sizeof(unsigned int) * 3;
  case UInt4:
    return sizeof(unsigned int) * 4;

  case Float:
    return sizeof(float);
  case Float2:
    return sizeof(float) * 2;
  case Float3:
    return sizeof(float) * 3;
  case Float4:
    return sizeof(float) * 4;

  case Mat3:
    return sizeof(glm::mat3);
  case Mat4:
    return sizeof(glm::mat4);

  default:
    UNREACHABLE("Undefined shader data type");
  }

  return 0;
}

inline constexpr u32 ShaderDataTypeComponentCount(ShaderDataType type) {
  using enum ShaderDataType;
  switch (type) {
  case Bool:
    return 1;

  case Int:
    return 1;
  case Int2:
    return 2;
  case Int3:
    return 3;
  case Int4:
    return 4;

  case UInt:
    return 1;
  case UInt2:
    return 2;
  case UInt3:
    return 3;
  case UInt4:
    return 4;

  case Float:
    return 1;
  case Float2:
    return 2;
  case Float3:
    return 3;
  case Float4:
    return 4;

  case Mat3:
    return 3 * 3;
  case Mat4:
    return 4 * 4;

  default:
    UNREACHABLE("Undefined shader data type component count");
    return 0;
  }
}

struct TextureBinding {
  Assets::AssetHandle<Texture2D> texture;
  Assets::AssetHandle<Sampler> sampler;
};

using ShaderParamValue =
    std::variant<bool, i32, u32, f32, glm::vec2, glm::vec3, glm::vec4,
                 glm::mat3, glm::mat4, TextureBinding>;

struct ShaderParameter {
  std::string name;
  ShaderDataType type;
};

} // namespace Jupiter::Rendering::RHI
