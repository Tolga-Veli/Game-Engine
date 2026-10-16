#pragma once

#include "DrawCommand.hpp"
#include "SceneData.hpp"

struct GLFWwindow;

namespace Jupiter::Rendering::RHI {

enum class VSyncMode : u8 { Off, On, Adaptive };

class IRenderer {
public:
  virtual ~IRenderer() = default;

  virtual void Init(GLFWwindow *window) = 0;
  virtual void Shutdown() = 0;

  virtual void BeginFrame(const SceneData &sceneData) = 0;
  virtual void EndFrame() = 0;

  virtual void Submit(const DrawCommand &cmd) = 0;

  virtual void SetVSync(VSyncMode mode) = 0;
};
}; // namespace Jupiter::Rendering::RHI
