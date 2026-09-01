#pragma once

#include "Core/Utils.hpp"

struct GLFWwindow;

namespace Jupiter::Rendering::RHI {

enum class VSyncMode : u8 { Off, On, Adaptive };

class IGraphicsContext {
public:
  virtual ~IGraphicsContext() = default;

  static Scope<IGraphicsContext> Create(GLFWwindow *nativeWindow);

  virtual bool Init() = 0;
  virtual void Shutdown() = 0;

  virtual void SetVSync(VSyncMode mode) = 0;
  virtual VSyncMode GetVSync() const noexcept = 0;

  virtual void OnFrame() = 0;
};
} // namespace Jupiter::Rendering::RHI
