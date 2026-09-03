#pragma once

#include <glm/glm.hpp>

namespace Jupiter::Rendering {
struct SceneData {
  glm::mat4 viewProjection{1.0f};
  glm::ivec2 framebufferSize{};
};

} // namespace Jupiter::Rendering
