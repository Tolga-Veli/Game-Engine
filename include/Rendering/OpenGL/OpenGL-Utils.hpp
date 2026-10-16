#pragma once

#include "Core/Assert.hpp"
#include "Rendering/Backend/Render-Utils.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
inline constexpr GLenum JupiterToOpenGLShaderDataType(RHI::DataType type) {
  using enum RHI::DataType;
  switch (type) {
  case Bool:
  case Int:
  case Int2:
  case Int3:
  case Int4:
    return GL_INT;

  case UInt:
  case UInt2:
  case UInt3:
  case UInt4:
    return GL_UNSIGNED_INT;

  case Float:
  case Float2:
  case Float3:
  case Float4:
  case Mat3:
  case Mat4:
    return GL_FLOAT;

  case Texture2D:
  case None:
  case Count:
    return 0;
  }

  UNREACHABLE("Undefined shader data type");
  return 0;
}

inline constexpr u32 JupiterToOpenGLShaderType(RHI::ShaderType type) {
  using enum RHI::ShaderType;
  switch (type) {
  case Vertex:
    return GL_VERTEX_SHADER;
  case Fragment:
    return GL_FRAGMENT_SHADER;
  case Geometry:
    return GL_GEOMETRY_SHADER;
  case Compute:
    return GL_COMPUTE_SHADER;

  case None:
  case TessEval:
  case TessControl:
  case Count:
    return 0;
  }

  UNREACHABLE("Undefined shader type");
  return 0;
}

inline GLenum JupiterToOpenGLTextureFormat(RHI::TextureFormat format) noexcept {
  using enum RHI::TextureFormat;
  switch (format) {
  case None:
    return GL_NONE;
  case R8:
    return GL_RED;
  case RG8:
    return GL_RG;
  case RGB8:
    return GL_RGB;
  case RGBA8:
    return GL_RGBA;
  }
  return GL_NONE;
}

inline GLenum JupiterToOpenGLInternalTextureFormat(RHI::TextureFormat format) noexcept {
  using enum RHI::TextureFormat;
  switch (format) {
  case R8:
    return GL_R8;
  case RG8:
    return GL_RG8;
  case RGB8:
    return GL_RGB8;
  case RGBA8:
    return GL_RGBA8;
  case None:
    return GL_NONE;
  }
  return GL_NONE;
}

inline GLenum JupiterToOpenGLFilterMode(RHI::FilterMode filter) noexcept {
  using enum RHI::FilterMode;
  switch (filter) {
  case Nearest:
    return GL_NEAREST;
  case Linear:
    return GL_LINEAR;
  }
  return GL_NONE;
}

inline GLenum JupiterToOpenGLWrapMode(RHI::WrapMode wrap) noexcept {
  using enum RHI::WrapMode;
  switch (wrap) {
  case Repeat:
    return GL_REPEAT;
  case MirroredRepeat:
    return GL_MIRRORED_REPEAT;
  case ClampToEdge:
    return GL_CLAMP_TO_EDGE;
  case ClampToBorder:
    return GL_CLAMP_TO_BORDER;
  }
  return GL_NONE;
}

} // namespace Jupiter::Rendering::OpenGL
