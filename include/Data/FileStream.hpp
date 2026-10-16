#pragma once

#include <filesystem>
#include <fstream>

#include "Core/Logging.hpp"
#include "Core/Storage.hpp"

namespace Jupiter::Data {

enum class FileError : u8 {
  Success = 0,
  OpenFailed,
  SizeError,
  LoadFailed,
};

inline FileError ReadFile(const std::filesystem::path &path, Storage &buffer) {
  buffer.clear();
  LOG_INFO("Reading file: {}", std::filesystem::absolute(path).string());
  std::ifstream in(path, std::ios::in | std::ios::binary | std::ios::ate);

  if (!in)
    return FileError::OpenFailed;

  auto size = in.tellg();
  if (size <= 0)
    return FileError::SizeError;

  buffer = Storage(size);
  in.seekg(0, std::ios::beg);

  if (!in.read(buffer.as<char>(), size)) {
    buffer.release();
    return FileError::LoadFailed;
  }

  return FileError::Success;
}
} // namespace Jupiter::Data
