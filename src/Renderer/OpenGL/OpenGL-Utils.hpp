#include "Renderer/Backend/RenderUtils.hpp"

#include <glad/gl.h>

namespace Jupiter::Renderer {
inline GLenum JupiterToOpenGLBaseType(ShaderDataType type) {
  using enum ShaderDataType;
  switch (type) {
  case Float:
  case Float2:
  case Float3:
  case Float4:
  case Mat3:
  case Mat4:
    return GL_FLOAT;
  case Int:
  case Int2:
    return GL_INT;
  default:
    return 0;
  }
}

inline GLenum JupiterToOpenGLBufferUsage(BufferUsage usage) {
  using enum BufferUsage;
  switch (usage) {
  case Static:
    return GL_STATIC_DRAW;
  case Dynamic:
    return GL_DYNAMIC_DRAW;
  case Stream:
    return GL_STREAM_DRAW;
  default:
    return 0;
  }
};

inline constexpr u32 JupiterToOpenGLShaderStage(ShaderType type) {
  switch (type) {
  case ShaderType::Vertex:
    return GL_VERTEX_SHADER;
  case ShaderType::Fragment:
    return GL_FRAGMENT_SHADER;
  case ShaderType::Geometry:
    return GL_GEOMETRY_SHADER;
  case ShaderType::Compute:
    return GL_COMPUTE_SHADER;
  default:
    return 0;
  }
}
} // namespace Jupiter::Renderer
