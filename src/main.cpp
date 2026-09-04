#include <App.hpp>
#include <iostream>

int main() {
  std::cerr << "test\n";
  App app;
  if (!app.init())
    return 1;

  app.run();
  app.shutdown();
}
