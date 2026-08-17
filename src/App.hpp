#pragma once
#include <Logging.hpp>
#include <Renderer.hpp>
// #include <imgui.h>
#include <filesystem>
#include <fstream>
#include <chrono>

struct LogFileWriterData {
  std::chrono::year_month_day last_day;
  std::string filepath;
  std::ofstream file;
};

class App {
private:
  static bool translations_initialised;
  static Log::write_fun log_file_writer;
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

  static void init_translations();
  static bool set_directory_globals();
  static bool make_directories(const std::filesystem::path &path);

  bool init();
  void run();
  void shutdown();
};
