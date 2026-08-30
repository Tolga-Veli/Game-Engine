#pragma once

#include "Core/Utils.hpp"

struct GLFWwindow;

namespace Jupiter::Rendering::RHI {

enum class VSyncMode : u8 { Off, On, Adaptive };

class IGraphicsContext {
public:
  virtual ~IGraphicsContext() = default;

  // Makes the context's resources current / ready to render with. Seperate from
  // construction so failures (missing driver support, function loader failures,
  // etc.) can be reported and handled instead of blowing up mid-ctor

  static Scope<IGraphicsContext> Create(GLFWwindow *handle);

  virtual bool Init() = 0;

  virtual void SetVSync(VSyncMode mode) = 0;
  virtual VSyncMode SetVSync() const = 0;

  virtual void SwapBuffers() = 0;
};
} // namespace Jupiter::Rendering::RHI
