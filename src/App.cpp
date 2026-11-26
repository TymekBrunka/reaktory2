#include <App.hpp>
#include <FileUtils.hpp>
#include <Logging.hpp>
#include <Renderer.hpp>
#include <Translations.hpp>

#include <FontsAwesome/IconsFontAwesome6.h>
#include <FontsAwesome/RobotoRegular.h>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <icon_img.h>
#include <images.h>
#include <images_bigger.h>
#include <imgui.h>
#include <iostream>
// #include <stdatomic.h>
#include "pfd/pfd.hpp"
#include <cstring>
#include <string>
#include <string_view>

#include "GLFW/glfw3.h"
#include "LogFileWriter.cpp"
#include "ImGuiConsoleLogger.cpp"
#include "Scene.hpp"

#include "pfd/pfd.hpp"
#include <zip.h>

// #include "App_gui.cpp"
// #include "App_popups.cpp"

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

    Log::Callback LogImGui_Callback{.tag = Log::ACCEPT_ALL & (~Log::VERBOSE),
                                    .data = &file_logger_data,
                                    .write = log_imgui_writer};

    logger = Log::Logger{
        Log::LANG_PL,
        {Log::ConsoleLog_Callback, LogFile_Callback, LogImGui_Callback}};
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
    if (app->does_have_modal() || !app->is_scene_window_selected()) {
      app->input.front = false;
      app->input.back = false;
      app->input.left = false;
      app->input.right = false;
      app->input.up = false;
      app->input.down = false;
      app->set_movement_vec();
    } else if (app->is_scene_window_selected() &&
               (mods == GLFW_MOD_SHIFT || mods == 0)) {
      if (action == GLFW_PRESS && app->get_selected_scene()) {
        switch (key) {
        case GLFW_KEY_W:
          app->input.front = true;
          break;
        case GLFW_KEY_S:
          app->input.back = true;
          break;
        case GLFW_KEY_A:
          app->input.left = true;
          break;
        case GLFW_KEY_D:
          app->input.right = true;
          break;
        case GLFW_KEY_SPACE:
          app->input.up = true;
          break;
        case GLFW_KEY_LEFT_SHIFT:
          app->input.down = true;
          break;
        }
        app->set_movement_vec();
      } else if (action == GLFW_RELEASE && app->get_selected_scene()) {
        switch (key) {
        case GLFW_KEY_W:
          app->input.front = false;
          break;
        case GLFW_KEY_S:
          app->input.back = false;
          break;
        case GLFW_KEY_A:
          app->input.left = false;
          break;
        case GLFW_KEY_D:
          app->input.right = false;
          break;
        case GLFW_KEY_SPACE:
          app->input.up = false;
          break;
        case GLFW_KEY_LEFT_SHIFT:
          app->input.down = false;
        }
        app->set_movement_vec();
      }
    }
    if (action == GLFW_PRESS && app->get_selected_scene()) {
      if (key == GLFW_KEY_DELETE) {
        if (app->get_selected_scene()->selected_object !=
            app->get_selected_scene()
                ->prefabs[app->get_selected_scene()->current_prefab]
                .obj)
          if (app->get_selected_scene()->objPool.get(
                  app->get_selected_scene()->selected_object)) {
            app->get_selected_scene()->objPool.remove(
                app->get_selected_scene()->selected_object);
          }
      }

      if (mods == GLFW_MOD_CONTROL && key == GLFW_KEY_D) {
        objH so = app->get_selected_scene()->selected_object;
        auto &objPool = app->get_selected_scene()->objPool;
        if (so != objPool.rootH()) {
          objH clone = objPool.clone(so);
          objPool.get(objPool.get(so)->parent)->children.push_back(clone.idx);
          app->get_selected_scene()->selected_object = clone;
        }
      }

      if (mods == GLFW_MOD_ALT && key == GLFW_KEY_D) {
        app->add_model();
      }

      if (mods == GLFW_MOD_ALT && key == GLFW_KEY_F) {
        app->add_texture();
      }
    }

    if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
      switch (key) {
      case GLFW_KEY_N:
        app->add_scene();
        break;
      case GLFW_KEY_O:
        app->open_scene();
        break;
      case GLFW_KEY_S:
        for (auto &[name, scene] : app->scenes)
          scene.save();
        break;
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
          .value();
  new_scene_tex = render.LoadTexture(new_scene_img).value();

  icon_tex = render.LoadTexture(icon).value();
  icons = render.LoadTexture(icons_, true).value();
  keybinds_tex = render.LoadTexture(keybind_icons, true).value();

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

void App::add_model() {
  std::vector<std::string> models_to_load =
      ipfd::open_file("Wybór modeli", "",
                      {"Plik modelu (obj/m3d/fbx)", "*.obj;*.m3d;*.fbx",
                       "Wszystkie pliki", "*"},
                      ipfd::opt::multiselect);

  for (const auto &model : models_to_load) {
    auto e1 = selected_scene->resMan.ImportModel(
        FileUtils::RealFs{std::filesystem::path{}},
        std::filesystem::path{model});
  }
}

void App::add_texture() {
  std::vector<std::string> textures_to_load =
      ipfd::open_file("Wybór obrazka", "",
                      {"Plik obrazka (png/jp(e)g/gif)",
                       "*.png;*.jpg;*.jpeg;*.gif", "Wszystkie pliki", "*"},
                      ipfd::opt::multiselect);

  for (const auto &texture : textures_to_load) {
    auto e1 = selected_scene->resMan.ImportTexture(
        FileUtils::RealFs{std::filesystem::path{}},
        std::filesystem::path{texture});
  }
}

bool App::add_scene(const std::string &name) {
  if (name.empty()) {
    current_modal = "Nowa scena";
    return true;
  }

  if (std::filesystem::exists(FileUtils::APP_ROOT / "scenes" / name)) {
    return false;
  }

  try {
    scenes.emplace(name, name);
    std::string cpy;
    if (selected_scene_idx)
      cpy = *selected_scene_idx;

    if (!scenes[name].init(render)) {
      std::cerr << "failed to init scene\n";
      return false;
    }

    if (selected_scene_idx) {
      auto iter = scenes.find(cpy);
      selected_scene_idx = &iter->first;
      selected_scene = &iter->second;
    }
  } catch (const std::exception &err) {
    std::cerr << "Error: " << err.what() << "\n";
    return false;
  }

  return true;
}

bool App::open_scene(const std::string &name) {
  if (name.empty()) {
    current_modal = "Wybór sceny";
    return true;
  }

  std::u8string name_ = std::u8string(name.begin(), name.end());
  std::cerr << (FileUtils::APP_ROOT / "scenes" / name_) << "\n";
  if (!std::filesystem::exists(FileUtils::APP_ROOT / "scenes" / name_)) {
    return false;
  }

  try {
    scenes.emplace(name, name);
    std::string cpy;
    if (selected_scene_idx)
      cpy = *selected_scene_idx;

    if (!scenes[name].init(render)) {
      std::cerr << "failed to init scene\n";
      return false;
    }

    if (selected_scene_idx) {
      auto iter = scenes.find(cpy);
      selected_scene_idx = &iter->first;
      selected_scene = &iter->second;
    }
  } catch (const std::exception &err) {
    std::cerr << "Error: " << err.what() << "\n";
    return false;
  }

  return true;
}

std::expected<bool, int> App::export_scene() {
  if (!selected_scene)
    return false;

  std::string zip_to_save =
      ipfd::save_file("Eksportowanie sceny", "",
                      {"archiwum zip", "*.zip", "Wszystkie pliki", "*"});

  if (FileUtils::FileExists(zip_to_save)) {
    auto e1 = FileUtils::WriteFile(zip_to_save, "", sizeof(""));
  }

  zip_error_t error;
  zip_error_init(&error);
  zip_source_t *z_src = zip_source_buffer_create("", 0, 0, &error);

  zip_t *z;
  z = zip_open_from_source(z_src, ZIP_CREATE, &error);
  if (!z) {
    std::cerr << "NOT OK: " << zip_error_strerror(&error) << "\n";
    return std::unexpected(2);
  }

  // to keep data after zip_close
  zip_source_keep(z_src);

  std::vector<char *> data;
  size_t curfold_len = selected_scene->get_folder().u8string().size();
  for (auto &ent : std::filesystem::recursive_directory_iterator(
           selected_scene->get_folder())) {

    if (!std::filesystem::is_regular_file(ent))
      continue;

    auto res_fs = FileUtils::ReadFile(ent.path());
    if (!res_fs.has_value()) {
      std::string path = selected_scene->get_folder().string();
      switch (res_fs.error()) {
      case -2:
        Log::log(Log::ERROR | Log::SEV_MED, 0, "App",
                 TL(MSG_GENERIC_FILE_NOT_FOUND), std::make_format_args(path));
        break;

      case -1:
        Log::log(Log::ERROR | Log::SEV_MED, 0, "App",
                 TL(MSG_GENERIC_OPEN_ERROR), std::make_format_args(path));
        break;

      case 1:
        Log::log(Log::ERROR | Log::SEV_MED, 0, "App",
                 TL(MSG_GENERIC_READ_ERROR), std::make_format_args(path));
        break;

      default:
        break;
      }
      zip_error_fini(&error);
      zip_close(z);
      for (char *src : data)
        delete[] src;
      return std::unexpected(4);
    }

    std::u8string path = ent.path().u8string();
    path = path.substr(curfold_len + 1, path.size() - curfold_len);
#ifdef _WIN32
    for (auto &c : path) {
      if (*(char *)&c == '\\')
        *(char *)&c = '/';
    }
#endif
    std::cerr << "path is " << std::string(path.begin(), path.end()) << "\n";
    zip_source_t *source =
        zip_source_buffer(z, res_fs.value().data, res_fs.value().length, 0);
    data.push_back(res_fs.value().data);

    std::string path_ = std::string(path.begin(), path.end());
    if (zip_file_add(z, path_.c_str(), source, ZIP_FL_ENC_UTF_8) < 0) {
      zip_source_free(source);
      zip_error_fini(&error);
      zip_close(z);
      for (char *src : data)
        delete[] src;
      std::cerr << "zip_file_add(): " << zip_strerror(z) << "\n";
      return std::unexpected(5);
    }
  }
  // zip_source_t *source  = zip_source_buffer(z, "Cześć", sizeof("Cześć"), 0);
  // zip_file_add(z, "test.txt", source, ZIP_FL_ENC_UTF_8);

  if (zip_close(z) < 0) {
    std::cerr << "Can't close zip archive: " << zip_strerror(z) << "\n";
    zip_error_fini(&error);
    for (char *src : data)
      delete[] src;
    return std::unexpected(6);
  }

  zip_error_fini(&error);
  for (char *src : data)
    delete[] src;

  zip_stat_t stat;
  if (zip_source_stat(z_src, &stat) < 0) {
    std::cerr << "Can't stat source (zip archive): "
              << zip_error_strerror(zip_source_error(z_src)) << "\n";
    zip_source_free(z_src);
    return std::unexpected(7);
  }

  if (zip_source_open(z_src) < 0) {
    std::cerr << "Can't open source (zip archive): "
              << zip_error_strerror(zip_source_error(z_src)) << "\n";
    zip_source_free(z_src);
    return std::unexpected(8);
  }

  char *buffer = new char[stat.size];

  if ((zip_uint64_t)zip_source_read(z_src, buffer, stat.size) < stat.size) {
    std::cerr << "Can't read data from source (zip archive): "
              << zip_error_strerror(zip_source_error(z_src)) << "\n";
    zip_source_free(z_src);
    delete[] buffer;
    return std::unexpected(9);
  }
  zip_source_free(z_src);

  auto res_fs2 = FileUtils::WriteFile(zip_to_save, buffer, stat.size);
  delete[] buffer;

  if (!res_fs2.has_value()) {
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
             res_fs2.error() == -1 ? TL(MSG_GENERIC_OPEN_ERROR)
                                   : TL(MSG_GENERIC_WRITE_ERROR),
             std::make_format_args(zip_to_save));
    return std::unexpected(10);
  }

  return true;
}
