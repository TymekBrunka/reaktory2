#include <App.hpp>
#include <FileUtils.hpp>
#include <Logging.hpp>
#include <Renderer.hpp>
#include <Translations.hpp>

#include <FontsAwesome/IconsFontAwesome6.h>
#include <FontsAwesome/RobotoRegular.h>
#include <cstdlib>
#include <icon_img.h>
#include <images.h>
#include <images_bigger.h>
#include <imgui.h>
#include <iostream>
// #include <stdatomic.h>
#include <cstring>

#include "GLFW/glfw3.h"
#include "LogFileWriter.cpp"
#include "Scene.hpp"

#include "App_gui.cpp"
#include "App_popups.cpp"

static char name_buffer[200] = {0};
static std::string owned_string;

// [app statup] -----
// //
bool App::translations_initialised = false;
bool App::logger_initialised = false;
LogFileWriterData App::file_logger_data{};
Log::Logger App::logger{};

bool App::init_translations() {
  if (!translations_initialised) {
    if (!Log::LoadTranslation(&Log::messages[Log::LANG_EN],
                              "assets/translations/en_US.json"))
      return false;

    if (!Log::LoadTranslation(&Log::messages[Log::LANG_PL],
                              "assets/translations/pl_PL.json"))
      return false;
    translations_initialised = true;

    return true;
  }
  return true;
}

bool App::set_directory_globals() {
#ifdef _WIN32
  const char *home_dir = getenv("USERPROFILE");
#else
  const char *home_dir = getenv("HOME");
#endif

  if (!home_dir) {
    std::cerr << Log::messages[Log::LANG_PL][TL(MSG_APP_HOME_DIR_ERROR)]
              << "\n";
  }

  FileUtils::HOME_DIR = home_dir;
  FileUtils::APP_ROOT = FileUtils::HOME_DIR / ".reaktory";
  return true;
}

bool App::make_directories(const std::filesystem::path &path) {
  try {
    std::filesystem::create_directory(path);
    std::filesystem::create_directory(path / "scenes");
    std::filesystem::create_directory(path / "tmp");
    std::filesystem::create_directory(path / "logs");
  } catch (std::filesystem::filesystem_error &err) {
    std::string path = err.path1().string();
    std::cerr << LOG_FMT(Log::LANG_PL, TL(MSG_APP_ROOT_SUBDIR_CREATE_ERROR),
                         Log::, std::make_format_args(path), true)
              << "\n";
    return false;
  }
  return true;
}

void App::init_logger() {
  if (!logger_initialised) {
    // Log::ConsoleLog_Callback.tag = Log::ACCEPT_ALL;
    Log::Callback LogFile_Callback{.tag = Log::ACCEPT_ALL,
                                   .data = &file_logger_data,
                                   .write = log_file_writer};

    logger =
        Log::Logger{Log::LANG_PL, {Log::ConsoleLog_Callback, LogFile_Callback}};
    Log::Logger::Global = &logger;
    logger_initialised = true;
  }
}
// \\
// [app statup] -----

bool App::init() {
  if (!init_translations()) {
    std::cerr << "Nie udało się załadować tłumaczeń\n";
    return false;
  }

  if (!set_directory_globals())
    return false;

  if (!make_directories(FileUtils::APP_ROOT))
    return false;

  init_logger();

  if (!Renderer::Render::Init())
    return false;

  if (!render.init("Reaktory", {960, 540}))
    return false;

  if (!Scene::Init(render))
    return false;

  render.userdata = this;

  render.SetResizeCallback([](Renderer::Render &window, int width, int height) {
    App *app = (App *)window.userdata;
    app->draw_self();
  });

  render.SetKeyCallback([](Renderer::Render &window, int key, int scancode,
                           int action, int mods) {
    App *app = (App *)window.userdata;
    if (app->does_have_modal())
      return;

    if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
      switch (key) {
      case GLFW_KEY_N:
        app->add_scene();
        break;
      case GLFW_KEY_O:
        app->open_scene();
        break;
      }
    } else if (action == GLFW_PRESS && app->get_selected_scene()) {
      switch (key) {
      case GLFW_KEY_W:
        app->movement_input.x += 1;
        break;
      case GLFW_KEY_S:
        app->movement_input.x += -1;
        break;
      case GLFW_KEY_A:
        app->movement_input.y += -1;
        break;
      case GLFW_KEY_D:
        app->movement_input.y += 1;
        break;
      case GLFW_KEY_SPACE:
        app->movement_input.z += 1;
        break;
      case GLFW_KEY_LEFT_SHIFT:
        app->movement_input.z += -1;
      }
    } else if (action == GLFW_RELEASE && app->get_selected_scene()) {
      switch (key) {
      case GLFW_KEY_W:
        app->movement_input.x -= 1;
        break;
      case GLFW_KEY_S:
        app->movement_input.x -= -1;
        break;
      case GLFW_KEY_A:
        app->movement_input.y -= -1;
        break;
      case GLFW_KEY_D:
        app->movement_input.y -= 1;
        break;
      case GLFW_KEY_SPACE:
        app->movement_input.z -= 1;
        break;
      case GLFW_KEY_LEFT_SHIFT:
        app->movement_input.z -= -1;
      }
    }
  });

  render.SetMouseButtonCallback([](Renderer::Render &window, int button,
                                   int action, int mods) {
    App *app = (App *)window.userdata;

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
      glfwSetInputMode(window.GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      app->mousebuttonR = false;
      return;
    }

    if (!app->is_scene_window_selected() || !app->get_selected_scene()) {
      app->mousebuttonL = false;
      app->mousebuttonR = false;
      return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
      app->mousebuttonL = true;

    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
      app->mousebuttonL = false;

    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
      glfwSetInputMode(window.GetGLFWWindow(), GLFW_CURSOR,
                       GLFW_CURSOR_DISABLED);
      app->mousebuttonR = true;
    }
  });

  ImFontConfig fontcfg;
  // fontcfg.PixelSnapH = true;
  // fontcfg.FontDataOwnedByAtlas = false;
  fontcfg.MergeMode = true;

  static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->AddFontFromMemoryCompressedTTF(
      RobotoRegular_compressed_data, RobotoRegular_compressed_size, 16.0f);
  io.Fonts->AddFontFromMemoryCompressedTTF(
      FA_compressed_data, FA_compressed_size, 16.0f, &fontcfg, icons_ranges);

  Renderer::Image icon{.width = icon_png_width,
                       .height = icon_png_height,
                       .channels = icon_png_channels,
                       .mipmap_levels = 0,
                       .pixels = (unsigned char *)icon_png_pixels};

  Renderer::Image icons_{.width = icons_png_width,
                         .height = icons_png_height,
                         .channels = icons_png_channels,
                         .mipmap_levels = 0,
                         .pixels = (unsigned char *)icons_png_pixels};

  Renderer::Image keybind_icons{.width = keybind_icons_png_width,
                                .height = keybind_icons_png_height,
                                .channels = keybind_icons_png_channels,
                                .mipmap_levels = 0,
                                .pixels =
                                    (unsigned char *)keybind_icons_png_pixels};

  new_scene_img =
      render.LoadImageFromMemory(scene_new_png_data, scene_new_png_size)
          .ok_unchecked();
  new_scene_tex = render.LoadTexture(new_scene_img).ok_unchecked();

  icon_tex = render.LoadTexture(icon).ok_unchecked();
  icons = render.LoadTexture(icons_, true).ok_unchecked();
  keybinds_tex = render.LoadTexture(keybind_icons, true).ok_unchecked();

  render.SetWindowIcon(icon);

  return true;
}

void App::shutdown() {
  free(new_scene_img.pixels);
  render.UnloadTexture(new_scene_tex);
  render.UnloadTexture(icon_tex);
  render.UnloadTexture(icons);
  render.UnloadTexture(keybinds_tex);

  render.cleanup();
  Renderer::Render::Cleanup();
}

void App::run() {
  render.makeContextCurrent();
  while (!render.WindowShouldClose()) {
    Renderer::Render::PullEvents();
    draw_self();
  }
}

void App::draw_self() {
  if (render.IsMinimised())
    return;
  render.BeginFrame();
  ImGui::DockSpaceOverViewport();
  // ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
  //                              ImGuiDockNodeFlags_PassthruCentralNode);

  draw_gui();
  draw_popup();
  render.EndFrame();
}

bool App::add_scene(const std::string &name) {
  if (name.empty()) {
    current_modal = "Nowa scena";
    return true;
  }

  if (std::filesystem::exists(FileUtils::APP_ROOT / "scenes" / name)) {
    return false;
  }

  Scene scene;
  try {
    scene = std::move(Scene{name});
  } catch (const std::exception &err) {
    return false;
  }

  if (!scene.init(render)) {
    std::cerr << "failed to init scene\n";
    return false;
  }

  if (selected_scene_idx) {
    std::string cpy = *selected_scene_idx;
    scenes[name] = std::move(scene);
    auto iter = scenes.find(cpy);
    selected_scene_idx = &iter->first;
    selected_scene = &iter->second;
  } else {
    scenes[name] = std::move(scene);
  }

  return true;
}

bool App::open_scene(const std::string &name) {
  if (name.empty()) {
    current_modal = "Wybór sceny";
    return true;
  }

  if (!std::filesystem::exists(FileUtils::APP_ROOT / "scenes" / name)) {
    return false;
  }

  Scene scene;
  try {
    scene = std::move(Scene{name});
  } catch (const std::exception &err) {
    return false;
  }

  if (!scene.init(render)) {
    std::cerr << "failed to init scene\n";
    return false;
  }

  if (selected_scene_idx) {
    std::string cpy = *selected_scene_idx;
    scenes[name] = std::move(scene);
    auto iter = scenes.find(cpy);
    selected_scene_idx = &iter->first;
    selected_scene = &iter->second;
  } else {
    scenes[name] = std::move(scene);
  }

  return true;
}
