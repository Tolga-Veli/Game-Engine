#pragma once

#include <glm/glm.hpp>

namespace Jupiter::Rendering {
struct SceneData {
  glm::mat4 viewProjection{1.0f};
  glm::ivec2 framebufferSize{};
  glm::vec4 clearColor{0.1f, 0.1f, 0.1f, 1.0f};
};

} // namespace Jupiter::Rendering
