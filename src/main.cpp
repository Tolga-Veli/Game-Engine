#include "Core/Application.hpp"

class MyApp : public Jupiter::Application {
  // user defined application
};

int main() {
  MyApp app;
  app.Run();
}
