#pragma once

#include <string>
#include <filesystem>
#include <fstream>

#include "Core/Logging.hpp"

namespace Jupiter::Data {
inline std::string ReadFile(const std::filesystem::path &path) {
  LOG_INFO("Reading file: {}", std::filesystem::absolute(path).string());
  std::ifstream in(path, std::ios::in | std::ios::binary | std::ios::ate);

  if (!in) {
    LOG_ERROR("Couldn't open file: {}", path.string());
    return "";
  }

  auto size = in.tellg();
  if (size <= 0)
    return "";

  std::string result;
  result.resize(size);

  in.seekg(0, std::ios::beg);
  if (!in.read(result.data(), size)) {
    LOG_ERROR("Couldn't load file: {}", path.string());
    return "";
  }

  return result;
}
} // namespace Jupiter::Data
