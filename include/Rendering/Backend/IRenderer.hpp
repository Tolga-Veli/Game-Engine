#pragma once

#include "DrawCommand.hpp"
#include "SceneData.hpp"

namespace Jupiter::Core {
class Window;
}

namespace Jupiter::Rendering::RHI {

enum class VSyncMode : u8 { Off, On, Adaptive };

class IRenderer {
public:
  virtual ~IRenderer() = default;

  virtual void Init(Core::Window &window) = 0;
  virtual void Shutdown() = 0;

  virtual void BeginFrame(const SceneData &sceneData) = 0;
  virtual void EndFrame() = 0;

  virtual void Submit(const DrawCommand &cmd) = 0;
  virtual void InvalidateState() {}

  virtual void SetVSync(VSyncMode mode) = 0;
};
}; // namespace Jupiter::Rendering::RHI
