#pragma once

#include "Rendering/Backend/ITexture.hpp"
#include "Data/FileStream.hpp"

#include <stb_image.h>

namespace Jupiter::Assets {
class TextureParser {
public:
  static Rendering::RHI::TextureData Parse(const std::filesystem::path &path) {
    Storage buffer;
    auto code = Data::ReadFile(path, buffer);
    if (code != Data::FileError::Success) {
      LOG_ERROR("Failed to read file: {}", path.string());
      return {};
    }

    i32 width{}, height{}, channels{};
    stbi_set_flip_vertically_on_load(true);
    using ImageData = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
    ImageData data{stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(buffer.data()), buffer.size(), &width,
                                         &height, &channels, 0),
                   &stbi_image_free};

    if (!data) {
      LOG_ERROR("Failed to load texture '{}' : {}", path.string(), stbi_failure_reason());
      return {};
    }

    if (width <= 0 || height <= 0 || channels <= 0) {
      LOG_ERROR("Texture '{}' has invalid dimensions ({}x{}, {} channels)", path.string(), width, height, channels);
      return {};
    }

    const std::size_t sz = 1LL * width * height * channels;
    const auto format = Rendering::RHI::ToTextureFormat(channels);

    if (format == Rendering::RHI::TextureFormat::None) {
      LOG_ERROR("Texture format was None");
      return {};
    }

    Storage image_data(std::span<const std::byte>{reinterpret_cast<const std::byte *>(data.get()), sz}); // copies
    return Rendering::RHI::TextureData{.data = std::move(image_data),
                                       .width = static_cast<u32>(width),
                                       .height = static_cast<u32>(height),
                                       .format = format};
  }
};
} // namespace Jupiter::Assets
