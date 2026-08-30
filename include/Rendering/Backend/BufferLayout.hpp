#pragma once

#include "Core/Utils.hpp"
#include "Core/Assert.hpp"

#include <string>
#include <vector>

namespace Jupiter::Rendering::RHI {
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

struct BufferElement {
  std::string name;
  ShaderDataType type;
  u32 size, offset, location;
  bool normalized;

  BufferElement(ShaderDataType _type, const std::string &_name, u32 _location,
                bool _normalized = false)
      : name(_name), type(_type), size(ShaderDataTypeSize(_type)), offset(0),
        location(_location), normalized(_normalized) {}
};

class BufferLayout {
public:
  BufferLayout() = default;
  BufferLayout(std::initializer_list<BufferElement> data) : m_Elements(data) {
    CalculateOffsetsAndStride();
  }

  [[nodiscard]] u32 GetStride() const { return m_Stride; }
  [[nodiscard]] const std::vector<BufferElement> &GetElements() const {
    return m_Elements;
  }

private:
  std::vector<BufferElement> m_Elements;
  u32 m_Stride{};

  void CalculateOffsetsAndStride() {
    u32 offset{};
    for (auto &element : m_Elements) {
      element.offset = offset;
      offset += element.size;
    }
    m_Stride = offset;
  }
};

} // namespace Jupiter::Rendering::RHI
