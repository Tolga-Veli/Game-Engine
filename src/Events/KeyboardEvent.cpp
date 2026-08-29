#include "KeyboardEvent.hpp"

#include <GLFW/glfw3.h>

namespace Jupiter::Core {

KeyCode GLFWKeyToKeyCode(i32 glfwKey) {
  switch (glfwKey) {
  case GLFW_KEY_SPACE:
    return KeyCode::Space;
  case GLFW_KEY_APOSTROPHE:
    return KeyCode::Apostrophe;
  case GLFW_KEY_COMMA:
    return KeyCode::Comma;
  case GLFW_KEY_MINUS:
    return KeyCode::Minus;
  case GLFW_KEY_PERIOD:
    return KeyCode::Period;
  case GLFW_KEY_SLASH:
    return KeyCode::Slash;

  case GLFW_KEY_0:
    return KeyCode::Num0;
  case GLFW_KEY_1:
    return KeyCode::Num1;
  case GLFW_KEY_2:
    return KeyCode::Num2;
  case GLFW_KEY_3:
    return KeyCode::Num3;
  case GLFW_KEY_4:
    return KeyCode::Num4;
  case GLFW_KEY_5:
    return KeyCode::Num5;
  case GLFW_KEY_6:
    return KeyCode::Num6;
  case GLFW_KEY_7:
    return KeyCode::Num7;
  case GLFW_KEY_8:
    return KeyCode::Num8;
  case GLFW_KEY_9:
    return KeyCode::Num9;

  case GLFW_KEY_SEMICOLON:
    return KeyCode::Semicolon;
  case GLFW_KEY_EQUAL:
    return KeyCode::Equal;

  case GLFW_KEY_A:
    return KeyCode::A;
  case GLFW_KEY_B:
    return KeyCode::B;
  case GLFW_KEY_C:
    return KeyCode::C;
  case GLFW_KEY_D:
    return KeyCode::D;
  case GLFW_KEY_E:
    return KeyCode::E;
  case GLFW_KEY_F:
    return KeyCode::F;
  case GLFW_KEY_G:
    return KeyCode::G;
  case GLFW_KEY_H:
    return KeyCode::H;
  case GLFW_KEY_I:
    return KeyCode::I;
  case GLFW_KEY_J:
    return KeyCode::J;
  case GLFW_KEY_K:
    return KeyCode::K;
  case GLFW_KEY_L:
    return KeyCode::L;
  case GLFW_KEY_M:
    return KeyCode::M;
  case GLFW_KEY_N:
    return KeyCode::N;
  case GLFW_KEY_O:
    return KeyCode::O;
  case GLFW_KEY_P:
    return KeyCode::P;
  case GLFW_KEY_Q:
    return KeyCode::Q;
  case GLFW_KEY_R:
    return KeyCode::R;
  case GLFW_KEY_S:
    return KeyCode::S;
  case GLFW_KEY_T:
    return KeyCode::T;
  case GLFW_KEY_U:
    return KeyCode::U;
  case GLFW_KEY_V:
    return KeyCode::V;
  case GLFW_KEY_W:
    return KeyCode::W;
  case GLFW_KEY_X:
    return KeyCode::X;
  case GLFW_KEY_Y:
    return KeyCode::Y;
  case GLFW_KEY_Z:
    return KeyCode::Z;

  case GLFW_KEY_LEFT_BRACKET:
    return KeyCode::LeftBracket;
  case GLFW_KEY_BACKSLASH:
    return KeyCode::Backslash;
  case GLFW_KEY_RIGHT_BRACKET:
    return KeyCode::RightBracket;
  case GLFW_KEY_GRAVE_ACCENT:
    return KeyCode::GraveAccent;

  case GLFW_KEY_ESCAPE:
    return KeyCode::Escape;
  case GLFW_KEY_ENTER:
    return KeyCode::Enter;
  case GLFW_KEY_TAB:
    return KeyCode::Tab;
  case GLFW_KEY_BACKSPACE:
    return KeyCode::Backspace;
  case GLFW_KEY_INSERT:
    return KeyCode::Insert;
  case GLFW_KEY_DELETE:
    return KeyCode::Delete;

  case GLFW_KEY_RIGHT:
    return KeyCode::Right;
  case GLFW_KEY_LEFT:
    return KeyCode::Left;
  case GLFW_KEY_DOWN:
    return KeyCode::Down;
  case GLFW_KEY_UP:
    return KeyCode::Up;

  case GLFW_KEY_PAGE_UP:
    return KeyCode::PageUp;
  case GLFW_KEY_PAGE_DOWN:
    return KeyCode::PageDown;
  case GLFW_KEY_HOME:
    return KeyCode::Home;
  case GLFW_KEY_END:
    return KeyCode::End;

  case GLFW_KEY_CAPS_LOCK:
    return KeyCode::CapsLock;
  case GLFW_KEY_SCROLL_LOCK:
    return KeyCode::ScrollLock;
  case GLFW_KEY_NUM_LOCK:
    return KeyCode::NumLock;
  case GLFW_KEY_PRINT_SCREEN:
    return KeyCode::PrintScreen;
  case GLFW_KEY_PAUSE:
    return KeyCode::Pause;

  case GLFW_KEY_F1:
    return KeyCode::F1;
  case GLFW_KEY_F2:
    return KeyCode::F2;
  case GLFW_KEY_F3:
    return KeyCode::F3;
  case GLFW_KEY_F4:
    return KeyCode::F4;
  case GLFW_KEY_F5:
    return KeyCode::F5;
  case GLFW_KEY_F6:
    return KeyCode::F6;
  case GLFW_KEY_F7:
    return KeyCode::F7;
  case GLFW_KEY_F8:
    return KeyCode::F8;
  case GLFW_KEY_F9:
    return KeyCode::F9;
  case GLFW_KEY_F10:
    return KeyCode::F10;
  case GLFW_KEY_F11:
    return KeyCode::F11;
  case GLFW_KEY_F12:
    return KeyCode::F12;

  case GLFW_KEY_LEFT_SHIFT:
    return KeyCode::LeftShift;
  case GLFW_KEY_LEFT_CONTROL:
    return KeyCode::LeftControl;
  case GLFW_KEY_LEFT_ALT:
    return KeyCode::LeftAlt;
  case GLFW_KEY_LEFT_SUPER:
    return KeyCode::LeftSuper;

  case GLFW_KEY_RIGHT_SHIFT:
    return KeyCode::RightShift;
  case GLFW_KEY_RIGHT_CONTROL:
    return KeyCode::RightControl;
  case GLFW_KEY_RIGHT_ALT:
    return KeyCode::RightAlt;
  case GLFW_KEY_RIGHT_SUPER:
    return KeyCode::RightSuper;

  case GLFW_KEY_MENU:
    return KeyCode::Menu;

  default:
    return KeyCode::Unknown;
  }
}

} // namespace Jupiter::Core
