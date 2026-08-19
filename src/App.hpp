#pragma once
#include <Logging.hpp>
#include <Renderer.hpp>
#include <Scene.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <imgui.h>

struct LogFileWriterData {
  std::chrono::year_month_day last_day;
  std::string filepath;
  std::string filepath_en;
  std::ofstream file;
  std::ofstream file_en;
};

class App {
private:
  static bool translations_initialised;
  static bool logger_initialised;
  static Log::write_fun log_file_writer;

  bool has_modal = false;
  static Log::Logger logger;
  static LogFileWriterData file_logger_data;
  Renderer::rTexture2D icon_tex;
  Renderer::rTexture2D new_scene_tex;
  Renderer::rTexture2D icons;
  Renderer::Image new_scene_img;
  Renderer::Render render;

  std::vector<Scene> scenes;

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
  static void init_logger();

  inline bool does_have_modal() const {
    return has_modal;
  }

  bool IconMenuItem(int idx, const char *label);
  void AddIconToDrawlist(int idx, ImVec2 offset = ImVec2(0, 0));

  bool init();
  void run();
  void shutdown();

  void add_scene();
};
