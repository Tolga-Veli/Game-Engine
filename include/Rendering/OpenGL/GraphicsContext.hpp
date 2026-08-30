#pragma once

#include "Core/Assert.hpp"
#include "Rendering/Backend/IGraphicsContext.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Jupiter::Rendering::OpenGL {

class GraphicsContext final : public RHI::IGraphicsContext {
public:
  // Does not touch GL/GLFW state yet - just stores the window. Call Init()
  // once the window exists and you're ready to make its context current.

  explicit GraphicsContext(GLFWwindow *window) noexcept : m_Window(window) {
    CORE_ASSERT(window, "GraphicsContext: window handle is null");
  }

  bool Init() override {
    glfwMakeContextCurrent(m_Window);

    i32 status = gladLoadGL(glfwGetProcAddress);
    if (status == 0) {
      CORE_VERIFY(false, "GraphicsContext: gladLoadGL failed");
      return false;
    }

    LOG_INFO("OpenGL Context:");
    LOG_INFO("  Vendor:   {}",
             reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
    LOG_INFO("  Renderer: {}",
             reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    LOG_INFO("  Version:  {}",
             reinterpret_cast<const char *>(glGetString(GL_VERSION)));

#ifdef JPT_DEBUG
    SetupDebugOutput();
#endif

    /*    i32 width, height;
        glfwGetFramebufferSize(m_Window, &width, &height);
        glViewport(0, 0, width, height);*/
    return true;
  }

  void SetVSync(RHI::VSyncMode mode) override {
    CORE_ASSERT(mode != RHI::VSyncMode::Adaptive,
                "Adaptive VSync is not implemented yet");

    // NOTE: inherited behaviour - requesting Adaptive here falls through the ":
    // 0" branch (vsync off) once asserts are compiled out in release

    glfwSwapInterval(mode == RHI::VSyncMode::On ? 1 : 0);
    m_VSyncMode = mode;
  }

  RHI::VSyncMode SetVSync() const override { return m_VSyncMode; }

  void SwapBuffers() override { glfwSwapBuffers(m_Window); }

#ifdef J_DEBUG
  static void GLAPIENTRY OnGLDebugMessage(GLenum, GLenum type, GLuint,
                                          GLenum severity, GLsizei,
                                          const GLchar *message, const void *) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
      return;
    if (severity == GL_DEBUG_SEVERITY_HIGH || type == GL_DEBUG_TYPE_ERROR)
      LOG_ERROR("[GL] {}", message);
    else
      LOG_INFO("[GL] {}", message);
  }

  static void SetupDebugOutput() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OnGLDebugMessage, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);
  }
#endif

private:
  GLFWwindow *m_Window;
  RHI::VSyncMode m_VSyncMode = RHI::VSyncMode::Off;
};
} // namespace Jupiter::Rendering::OpenGL
