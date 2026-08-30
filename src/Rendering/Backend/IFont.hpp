#pragma once

#include "Core/Utils.hpp"

namespace Jupiter::Rendering {
class IFont {
public:
  static Scope<IFont> Create(const void *data, std::size_t fontSize,
                             float pixelHeight);

private:
};
} // namespace Jupiter::Rendering
