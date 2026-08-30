#pragma once

namespace Jupiter::Rendering {

// Rebuilds a dynamic quad-per-glyph buffer on every DrawText call. Fine
// for UI/HUD text -- a handful of short strings a frame;
class TextRenderer {};

} // namespace Jupiter::Rendering
