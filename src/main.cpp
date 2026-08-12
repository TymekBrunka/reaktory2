// #include <Logging/Logging.hpp>
#include <Renderer/Renderer.hpp>
#include <icon_img.h>
#include <imgui.h>

#include <FontsAwesome/IconsFontAwesome6.h>
#include <FontsAwesome/RobotoRegular.h>
#include <App.hpp>
#include <Components/FileUtils/FileUtils.hpp>
#include <Components/Allocators/RPmallocator.hpp>

int main() {
  App app;
  if (!app.init())
    return 1;

  app.run();
  app.shutdown();
  Allocators::RPmallocator<unsigned char> rpmallocator;
  FileUtils::ReadFile("./src/main.cpp", rpmallocator);
}
