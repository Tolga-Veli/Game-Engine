#include "Core/Application.hpp"

int main() {
  auto app = Jupiter::Core::Application::Create();
  app->Run();
}
