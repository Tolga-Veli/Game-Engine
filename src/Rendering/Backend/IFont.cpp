#include "Rendering/Backend/IFont.hpp"
#include <stb_truetype.h>

namespace Jupiter::Rendering::RHI {
Scope<IFont> IFont::Create(const void *data, std::size_t fontSize,
                           float pixelHeight) {
  (void)data;
  (void)fontSize;
  (void)pixelHeight;

  return {};
}
} // namespace Jupiter::Rendering::RHI
