#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Renderer {
/*
 * Static - Upload once, use many times
 * Dynamic - Upload every frame (UI, particles)
 * Stream - Upload, use once, discard
 */
enum class BufferUsage { None = 0, Static, Dynamic, Stream, Count };

enum class ShaderDataType : u32 {
  None = 0,
  Float,
  Float2,
  Float3,
  Float4,
  Mat3,
  Mat4,
  Int,
  Int2,
  Int3,
  Int4,
  Bool,

  Count
};

/* Shader Stages
 *
 */
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

enum class RendererAPI { None = 0, OpenGL, Count };

inline constexpr u32 GetShaderDataTypeSize(ShaderDataType type) {
  using enum ShaderDataType;
  switch (type) {
  case Float:
    return 4;
  case Float2:
    return 4 * 2;
  case Float3:
    return 4 * 3;
  case Float4:
    return 4 * 4;
  case Mat3:
    return 4 * 3 * 3;
  case Mat4:
    return 4 * 4 * 4;
  case Int:
    return 4;
  case Int2:
    return 4 * 2;
  default:
    return 0;
  }
}

inline constexpr u32 GetShaderDataTypeComponentCount(ShaderDataType type) {
  using enum ShaderDataType;
  switch (type) {
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
  case Int:
    return 1;
  case Int2:
    return 2;
  default:
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
    return "Unknown";
  }
}

} // namespace Jupiter::Renderer
