#pragma once

#include "Core/Assert.hpp"
#include "Core/Utils.hpp"

namespace Jupiter::Rendering {
/*
 * Static - Upload once, use many times
 * Dynamic - Upload every frame (UI, particles)
 * Stream - Upload, use once, discard
 */
enum class BufferUsage { None = 0, Static, Dynamic, Stream, Count };

enum class ShaderDataType : u32 {
  None = 0,
  Bool,
  Int,
  UInt,
  Float,
  Float2,
  Vec2,
  Vec3,
  Vec4,
  Mat3,
  Mat4,

  Count
};

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

enum class GraphicsAPI { None = 0, OpenGL, Vulkan, Metal, DX11, DX12, Count };

enum class BlendMode { None = 0, Alpha, Additive, Count };

[[nodiscard]]
constexpr u32 ShaderDataTypeSize(ShaderDataType type) {
  using enum ShaderDataType;
  switch (type) {
  case Bool:
    return sizeof(bool);
  case Int:
    return sizeof(i32);
  case UInt:
    return sizeof(u32);
  case Float:
    return sizeof(f32);

  case Vec2:
    return sizeof(glm::vec2);
  case Vec3:
    return sizeof(glm::vec3);
  case Vec4:
    return sizeof(glm::vec4);

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
  case Float:
    return 1;
  case Float2:
    return 2;
  case Mat3:
    return 3 * 3;
  case Mat4:
    return 4 * 4;
  case Int:
    return 1;
  case Bool:
    return 1;
  default:
    UNREACHABLE("Undefined shader data type component count");
    return 0;
  }
}

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

} // namespace Jupiter::Rendering
