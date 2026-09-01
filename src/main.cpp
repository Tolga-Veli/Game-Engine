#include "Core/Application.hpp"

class MyApp : public Jupiter::Application {
  // user defined application
};

int main() {
  auto app = Jupiter::Application::Create();
  app->Run();
}
