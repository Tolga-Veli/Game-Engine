#pragma once

#include <filesystem>

namespace Jupiter::Core {
class Audio {
public:
  Audio(const std::filesystem::path &path);
  ~Audio();

private:
};
} // namespace Jupiter::Core
