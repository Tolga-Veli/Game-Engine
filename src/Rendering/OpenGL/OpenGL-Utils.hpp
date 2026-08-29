#include "Core/Assert.hpp"
#include "Rendering/Backend/RenderUtils.hpp"
#include "Rendering/Backend/IFramebuffer.hpp"

#include <glad/gl.h>

namespace Jupiter::Rendering::OpenGL {
inline constexpr GLenum JupiterToOpenGLBaseType(ShaderDataType type) {
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
    UNREACHABLE("Undefined shader data type");
    return 0;
  }
}

inline constexpr GLenum JupiterToOpenGLBufferUsage(BufferUsage usage) {
  using enum BufferUsage;
  switch (usage) {
  case Static:
    return GL_STATIC_DRAW;
  case Dynamic:
    return GL_DYNAMIC_DRAW;
  case Stream:
    return GL_STREAM_DRAW;
  default:
    UNREACHABLE("Undefined buffer usage");
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
    UNREACHABLE("Undefined shader type");
    return 0;
  }
}

inline constexpr GLenum
JupiterToOpenGLFramebufferTextureFormat(FramebufferTextureFormat f) {
  switch (f) {
  case FramebufferTextureFormat::RGBA8:
    return GL_RGBA8;
  case FramebufferTextureFormat::RGBA16F:
    return GL_RGBA16F;
  case FramebufferTextureFormat::RedInteger:
    return GL_R32I;
  case FramebufferTextureFormat::Depth24Stencil8:
    return GL_DEPTH24_STENCIL8;
  default:
    UNREACHABLE("Undefined framebuffer texture format");
    return 0;
  }
}

} // namespace Jupiter::Rendering::OpenGL
