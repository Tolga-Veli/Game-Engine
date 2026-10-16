#pragma once

struct GLFWwindow;

namespace Jupiter::Rendering::RHI {

class IGraphicsContext {
public:
  virtual ~IGraphicsContext() = default;

  virtual bool Init() = 0;
  virtual void Shutdown() = 0;

  virtual void OnFrame() = 0;
};
} // namespace Jupiter::Rendering::RHI
