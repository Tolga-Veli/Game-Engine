#pragma once

#include "Core/Utils.hpp"

#include "Rendering/Backend/IFont.hpp"

#include <glm/glm.hpp>
#include <stb_truetype.h>

namespace Jupiter::Rendering {
class Font {
  ~Font() noexcept = default;

private:
  Scope<RHI::IFont> m_Impl;
};
} // namespace Jupiter::Rendering
