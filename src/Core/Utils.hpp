#pragma once

#include <cstdint>
#include <memory>
#include <glm/glm.hpp>

namespace Jupiter {
template <typename T> using Ref = std::shared_ptr<T>;
template <typename T> using Scope = std::unique_ptr<T>;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

template <typename T>
constexpr size_t EnumToSize(T value)
  requires(std::is_enum_v<T>)
{
  return static_cast<u32>(value);
}

namespace Core::Colors {
// RGB
inline constexpr glm::vec3 Black = {0.0f, 0.0f, 0.0f};
inline constexpr glm::vec3 White = {1.0f, 1.0f, 1.0f};

inline constexpr glm::vec3 Red = {1.0f, 0.0f, 0.0f};
inline constexpr glm::vec3 Green = {0.0f, 1.0f, 0.0f};
inline constexpr glm::vec3 Blue = {0.0f, 0.0f, 1.0f};

inline constexpr glm::vec3 Yellow = {1.0f, 1.0f, 0.0f};
inline constexpr glm::vec3 Cyan = {0.0f, 1.0f, 1.0f};
inline constexpr glm::vec3 Magenta = {1.0f, 0.0f, 1.0f};

inline constexpr glm::vec3 Gray = {0.5f, 0.5f, 0.5f};
inline constexpr glm::vec3 DarkGray = {0.25f, 0.25f, 0.25f};
inline constexpr glm::vec3 LightGray = {0.75f, 0.75f, 0.75f};

// RGBA
namespace RGBA {

inline constexpr glm::vec4 Black = {0.0f, 0.0f, 0.0f, 1.0f};
inline constexpr glm::vec4 White = {1.0f, 1.0f, 1.0f, 1.0f};

inline constexpr glm::vec4 Red = {1.0f, 0.0f, 0.0f, 1.0f};
inline constexpr glm::vec4 Green = {0.0f, 1.0f, 0.0f, 1.0f};
inline constexpr glm::vec4 Blue = {0.0f, 0.0f, 1.0f, 1.0f};

inline constexpr glm::vec4 Yellow = {1.0f, 1.0f, 0.0f, 1.0f};
inline constexpr glm::vec4 Cyan = {0.0f, 1.0f, 1.0f, 1.0f};
inline constexpr glm::vec4 Magenta = {1.0f, 0.0f, 1.0f, 1.0f};

inline constexpr glm::vec4 Gray = {0.5f, 0.5f, 0.5f, 1.0f};
inline constexpr glm::vec4 DarkGray = {0.25f, 0.25f, 0.25f, 1.0f};
inline constexpr glm::vec4 LightGray = {0.75f, 0.75f, 0.75f, 1.0f};

} // namespace RGBA

// convert the traditional [0,255] colours to [0,1]
constexpr glm::vec3 to_RGB(u8 r, u8 g, u8 b) {
  return {r / 255.0f, g / 255.0f, b / 255.0f};
}

constexpr glm::vec4 to_RGBA(u8 r, u8 g, u8 b, u8 a = 255) {
  return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}
} // namespace Core::Colors
} // namespace Jupiter
