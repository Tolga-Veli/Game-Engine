#pragma once

#include "Rendering/Backend/IGraphicsContext.hpp"
#include "Core/Logging.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Jupiter::Rendering::OpenGL {

// TODO: change graphics context to accept arbitrary window not just GLFW
class GraphicsContext final : public RHI::IGraphicsContext {
public:
  // Does not touch GL/GLFW state yet - just stores the window. Call Init()
  // once the window exists and you're ready to make its context current.
  explicit GraphicsContext(GLFWwindow *window) : m_Window(window) {
    if (!m_Window)
      throw std::runtime_error("GraphicsContext: GLFW window is nullptr");
  }

  bool Init() override {
    if (!m_Window)
      throw std::runtime_error("GraphicsContext: GLFW window is nullptr");

    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGL(glfwGetProcAddress))
      throw std::runtime_error("GraphicsContext: gladLoadGL failed");

    i32 width, height;
    glfwGetFramebufferSize(m_Window, &width, &height);
    glViewport(0, 0, width, height);

    LOG_INFO("OpenGL Context:");
    LOG_INFO("  Vendor:   {}", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
    LOG_INFO("  Renderer: {}", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    LOG_INFO("  Version:  {}", reinterpret_cast<const char *>(glGetString(GL_VERSION)));

#ifdef J_DEBUG
    SetupDebugOutput();
#endif
    return true;
  }

  void Shutdown() override {
    if (glfwGetCurrentContext() == m_Window)
      glfwMakeContextCurrent(nullptr);
  }

  void OnFrame() override { glfwSwapBuffers(m_Window); }

#ifdef J_DEBUG
  static void GLAPIENTRY OnGLDebugMessage(GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar *message,
                                          const void *) {
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
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  }
#endif

private:
  GLFWwindow *m_Window{};
};
} // namespace Jupiter::Rendering::OpenGL
