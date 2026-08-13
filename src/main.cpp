// #include <Logging/Logging.hpp>
#include <App.hpp>

int main() {
  App app;
  if (!app.init())
    return 1;

  app.run();
  app.shutdown();
}
