#ifdef TEST
#include <print>
#include <chrono>
#include <cmath>
#include <cassert>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

template <class T, std::size_t N> struct vec {
  std::array<T, N> storage{};

  T &operator()(std::size_t idx) {
    assert(idx < N);
    return storage[idx];
  }

  const T &operator()(std::size_t idx) const {
    assert(idx < N);
    return storage[idx];
  }

  T dot(const vec &other) const noexcept {
    T res{};
    for (std::size_t i{0}; i < N; ++i)
      res += storage[i] * other.storage[i];
    return res;
  }
};

template <class T, std::size_t N> struct mat {
  std::array<std::array<T, N>, N> storage{};

  T &operator()(std::size_t x, std::size_t y) {
    assert(x < N && y < N);
    return storage[x][y];
  }

  const T &operator()(std::size_t x, std::size_t y) const {
    assert(x < N && y < N);
    return storage[x][y];
  }

  mat operator*(const mat &other) const noexcept {
    mat res{};

    for (std::size_t i{0}; i < N; ++i)
      for (std::size_t j{0}; j < N; ++j)
        for (std::size_t k{0}; k < N; ++k)
          res(i, j) += storage[i][k] * other(k, j);

    return res;
  }
};

using Vec3 = vec<float, 3>;
using Vec4 = vec<float, 4>;
using Mat4 = mat<float, 4>;

constexpr float PI = 3.14159265358979323846f;

Mat4 identity() {
  Mat4 res{};
  res(0, 0) = 1.0f;
  res(1, 1) = 1.0f;
  res(2, 2) = 1.0f;
  res(3, 3) = 1.0f;
  return res;
}

Mat4 rotateX(float angle) {
  Mat4 res = identity();

  const float cosv = std::cos(angle), sinv = std::sin(angle);

  res(1, 1) = cosv;
  res(1, 2) = -sinv;

  res(2, 1) = sinv;
  res(2, 2) = cosv;

  return res;
}

Mat4 rotateY(float angle) {
  Mat4 res = identity();

  const float cosv = std::cos(angle), sinv = std::sin(angle);

  res(0, 0) = cosv;
  res(0, 2) = sinv;

  res(2, 0) = -sinv;
  res(2, 2) = cosv;

  return res;
}

Mat4 translate(float x, float y, float z) {
  Mat4 res = identity();

  res(0, 3) = x;
  res(1, 3) = y;
  res(2, 3) = z;

  return res;
}

Mat4 perspective(float fov, float aspect, float zNear, float zFar) {
  Mat4 res{};
  const float tanHalfFov = std::tan(fov * 0.5f);

  res(0, 0) = 1.0f / (aspect * tanHalfFov);
  res(1, 1) = 1.0f / tanHalfFov;

  res(2, 2) = -(zFar + zNear) / (zFar - zNear);
  res(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);

  res(3, 2) = -1.0f;
  return res;
}

constexpr i32 WIDTH = 1920, HEIGHT = 1080;
constexpr std::string TITLE = "Test";

constexpr float aspect = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
constexpr float sensitivity = 0.01f;

constexpr std::string_view VERTEX_SHADER = R"glsl(
  #version 330 core

  layout(location = 0) in vec3 aPos;

  uniform mat4 uModel;
  uniform mat4 uView;
  uniform mat4 uProj;

  void main()
  {
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
  }
)glsl";

constexpr std::string_view FRAGMENT_SHADER = R"glsl(
  #version 330 core

  out vec4 FragColor;

  uniform vec4 uColor;

  void main()
  {
    FragColor = uColor;
  }
)glsl";

// clang-format off
constexpr float vertices[] = {
                              // front 
                              -0.5f, -0.5f,  0.5f,  
                               0.5f, -0.5f,  0.5f,
                               0.5f,  0.5f,  0.5f,
                              -0.5f,  0.5f,  0.5f,
                              
                             // back
                              -0.5f, -0.5f, -0.5f,
                               0.5f, -0.5f, -0.5f,
                               0.5f,  0.5f, -0.5f,
                              -0.5f, 0.5f,  -0.5f};

constexpr u32 indices[] = {
  // front
  0,1,2,
  2,3,0,

  // right
  1,5,6,
  6,2,1,

  // back
  5,4,7,
  7,6,5,

  // left
  4,0,3,
  3,7,4,

  // top
  3,2,6,
  6,7,3,

  // bottom
  4,5,1,
  1,0,4
};
// clang-format on

bool mouseHeld = false, firstMouse = true;
double lastMouseX = 0.0, lastMouseY = 0.0;
float yaw = 0.0f, pitch = 0.0f;

static void KeyCallback(GLFWwindow *window, i32 key, i32 scancode, i32 action,
                        i32 mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);

  (void)scancode;
  (void)mods;
}

static void ResizeCallback(GLFWwindow *window, i32 width, i32 height) {
  glViewport(0, 0, width, height);

  (void)window;
}

static void MouseButtonCallback(GLFWwindow *window, i32 button, i32 action,
                                i32 mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      mouseHeld = true;
      firstMouse = true;
    } else if (action == GLFW_RELEASE)
      mouseHeld = false;
  }

  (void)window;
  (void)mods;
}

static void MouseCallback(GLFWwindow *window, double xpos, double ypos) {
  if (!mouseHeld)
    return;

  if (firstMouse) {
    lastMouseX = xpos;
    lastMouseY = ypos;
    firstMouse = false;
    return;
  }

  const float dx = static_cast<float>(xpos - lastMouseX),
              dy = static_cast<float>(ypos - lastMouseY);

  lastMouseX = xpos;
  lastMouseY = ypos;

  yaw += dx * sensitivity;
  pitch += dy * sensitivity;

  (void)window;
}

int main() {
  if (!glfwInit()) {
    std::println("Failed to initialize glfw!");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, TITLE.c_str(), nullptr, nullptr);

  if (!window) {
    std::println("Failed to craete glfw window");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  glfwSwapInterval(0); // disables VSync

  i32 version = gladLoadGL(glfwGetProcAddress);
  if (!version) {
    std::println("Glad failed to initialize!");
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  std::println("OpenGL: {}",
               reinterpret_cast<const char *>(glGetString(GL_VERSION)));

  std::println("Renderer: {}",
               reinterpret_cast<const char *>(glGetString(GL_RENDERER)));

  i32 framebufferWidth, framebufferHeight;
  glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

  glViewport(0, 0, framebufferWidth, framebufferHeight);

  glfwSetKeyCallback(window, KeyCallback);
  glfwSetFramebufferSizeCallback(window, ResizeCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetCursorPosCallback(window, MouseCallback);

  // Enable depth-testing for 3D
  glEnable(GL_DEPTH_TEST);

  i32 success;
  char infoLog[512];

  u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
  {
    const char *src = VERTEX_SHADER.data();
    const GLint length = static_cast<GLint>(VERTEX_SHADER.length());
    glShaderSource(vertexShader, 1, &src, &length);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
      std::println("Error: Vertex shader compilation failed\n {}", infoLog);
    }
  }

  u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  {
    const char *src = FRAGMENT_SHADER.data();
    const GLint length = static_cast<GLint>(FRAGMENT_SHADER.length());
    glShaderSource(fragmentShader, 1, &src, &length);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
      std::println("Error: Fragment shader compilation failed\n {}", infoLog);
    }
  }

  u32 shaderProgram = glCreateProgram();
  {
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      std::println("Error: Linking shader program failed\n, {}", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }

  u32 vbo, vao, ibo;
  {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    if (!vao || !vbo) {
      std::println("Failed to create OpenGL vertex objects!");

      glDeleteVertexArrays(1, &vao);
      glDeleteBuffers(1, &vbo);
      glDeleteBuffers(1, &ibo);
      glDeleteProgram(shaderProgram);

      glfwDestroyWindow(window);
      glfwTerminate();
      return -1;
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);
  }

  i32 modelLoc = glGetUniformLocation(shaderProgram, "uModel");
  i32 viewLoc = glGetUniformLocation(shaderProgram, "uView");
  i32 projLoc = glGetUniformLocation(shaderProgram, "uProj");
  i32 colorLoc = glGetUniformLocation(shaderProgram, "uColor");
  if (modelLoc == -1 || viewLoc == -1 || projLoc == -1 || colorLoc == -1) {
    std::println("Failed to find uniform 'uColor'!");

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  const Mat4 view = translate(0.0f, 0.0f, -3.0f),
             proj = perspective(60.0f * PI / 180.0f, aspect, 0.1f, 100.0f);
  {
    glUseProgram(shaderProgram);

    // GL_TRUE since OpenGL expects column-major layout
    // but this is row-major layout so u need to transpose each matrix before
    // doing anything to
    glUniformMatrix4fv(viewLoc, 1, GL_TRUE, &view.storage[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_TRUE, &proj.storage[0][0]);
  }

  using Clock = std::chrono::steady_clock;
  const auto start = Clock::now();

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    const float seconds =
        std::chrono::duration<float>(Clock::now() - start).count();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    const float redv = std::sin(seconds) * 0.5f + 0.5f,
                greenv = std::sin(seconds * 0.7f) * 0.5f + 0.5f,
                bluev = std::sin(seconds * 1.3f) * 0.5f + 0.5f;

    pitch = std::clamp(pitch, -1.5f, 1.5f);
    const Mat4 model = rotateY(yaw) * rotateX(pitch);

    glUniformMatrix4fv(modelLoc, 1, GL_TRUE, &model.storage[0][0]);

    glUniform4f(colorLoc, redv, greenv, bluev, 1.0f);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

    glfwSwapBuffers(window);
  }

  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);
  glDeleteProgram(shaderProgram);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
#endif
