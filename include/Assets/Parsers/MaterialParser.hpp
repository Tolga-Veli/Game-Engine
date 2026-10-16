#pragma once

#include "Rendering/Backend/IMaterial.hpp"
#include "Data/FileStream.hpp"

namespace Jupiter::Assets {
class MaterialParser {
public:
  [[nodiscard]] static Rendering::RHI::MaterialData Parse(const std::filesystem::path &path) {
    Storage buffer;
    auto code = Data::ReadFile(path, buffer);

    if (code != Data::FileError::Success) {
      LOG_ERROR("Failed to read file: {}", path.string());
      return {};
    }

    return Rendering::RHI::MaterialData{};
  }
};
} // namespace Jupiter::Assets
