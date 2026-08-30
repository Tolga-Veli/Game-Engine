#pragma once

#include "Core/Utils.hpp"
#include "Core/Logging.hpp"

#include "Rendering/Backend/IFont.hpp"

#include <glm/glm.hpp>
#include <unordered_map>
#include <fstream>
#include <stb_truetype.h>
#include <filesystem>
#include <vector>
#include <array>

namespace Jupiter::Rendering {
class Font {
  ~Font() noexcept = default;

private:
  Scope<IFont> m_Impl;
};
} // namespace Jupiter::Rendering
