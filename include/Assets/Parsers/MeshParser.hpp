#pragma once

#include "Rendering/Backend/IMesh.hpp"
#include "Data/FileStream.hpp"

namespace Jupiter::Assets {
class MeshParser {
public:
  [[nodiscard]] static Rendering::RHI::MeshDesc Parse(const std::filesystem::path &path) {
    Storage buffer;
    auto code = Data::ReadFile(path, buffer);

    if (code != Data::FileError::Success) {
      LOG_ERROR("Failed to read file: {}", path.string());
      return {};
    }

    return Rendering::RHI::MeshDesc{};
  }
};
} // namespace Jupiter::Assets
