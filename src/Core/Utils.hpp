#pragma once

#include <fstream>
#include <string>
#include <filesystem>
#include <cstdint>
#include <memory>

#include "Logging.hpp"

namespace Jupiter {
template <typename T> using Ref = std::shared_ptr<T>;
template <typename T> using Scope = std::unique_ptr<T>;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i32 = std::int32_t;
using i64 = std::int64_t;

template <typename T>
constexpr size_t EnumToSize(T value)
  requires(std::is_enum_v<T>)
{
  return static_cast<u32>(value);
}

namespace Utils {
inline std::string ReadFile(const std::filesystem::path &path) {
  std::ifstream in(path, std::ios::in | std::ios::binary | std::ios::ate);

  if (!in) {
    J_ERROR("Couldn't open file: {}", path.string());
    return "";
  }

  auto size = in.tellg();
  if (size <= 0)
    return "";

  std::string result;
  result.resize(size);

  in.seekg(0, std::ios::beg);
  if (!in.read(result.data(), size)) {
    J_ERROR("Couldn't load file: {}", path.string());
    return "";
  }

  return result;
}
} // namespace Utils
} // namespace Jupiter
