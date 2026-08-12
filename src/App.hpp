#pragma once
#include <Logging.hpp>
#include <Renderer/Renderer.hpp>
#include <imgui.h>

class App {
private:
  // bool is_running;
  Log::Logger logger;
  Renderer::Render render;
  Renderer::rTexture2D icon_tex;

public:
  App() = default;
  ~App() = default;
  App(const App &other) = delete;
  App &operator=(const App &other) = delete;
  App(App &&other) = delete;
  App &operator=(App &&other) = delete;

  bool init();
  void run();
  void shutdown();
};
