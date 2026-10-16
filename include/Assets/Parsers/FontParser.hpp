#pragma once

#include "Rendering/Backend/IFont.hpp"
#include "Data/FileStream.hpp"

namespace Jupiter::Assets {
class FontParser {
public:
  [[nodiscard]] static Rendering::RHI::FontData Parse(const std::filesystem::path &path,
                                                      const Rendering::RHI::FontOptions &opts) {
    Storage buffer;
    auto code = Data::ReadFile(path, buffer);

    if (code != Data::FileError::Success) {
      LOG_ERROR("Failed to read file: {}", path.string());
      return {};
    }

    return Rendering::RHI::FontData{.data = std::move(buffer), .opts = std::move(opts)};
  }
};
} // namespace Jupiter::Assets
