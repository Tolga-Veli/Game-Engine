#pragma once

#include "Core/Utils.hpp"
#include "Render-Utils.hpp"

#include <vector>

namespace Jupiter::Rendering::RHI {
struct BufferElement {
  std::string_view name;
  DataType type;
  u32 size, offset, location;
  bool normalized;

  BufferElement(DataType _type, std::string_view _name, u32 _location, bool _normalized = false)
      : name(_name), type(_type), size(DataTypeSize(_type)), offset(0), location(_location), normalized(_normalized) {}
};

class BufferLayout {
public:
  BufferLayout() = default;
  BufferLayout(std::initializer_list<BufferElement> data) : m_Elements(data) { CalculateOffsetsAndStride(); }

  [[nodiscard]] u32 GetStride() const { return m_Stride; }
  [[nodiscard]] const std::vector<BufferElement> &GetElements() const { return m_Elements; }

  auto begin() { return m_Elements.begin(); }
  auto begin() const { return m_Elements.begin(); }
  auto end() { return m_Elements.end(); }
  auto end() const { return m_Elements.end(); }

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
