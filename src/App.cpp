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

#include "GLFW/glfw3.h"
#include "LogFileWriter.cpp"

// [app statup] -----
// //
bool App::translations_initialised = false;
bool App::logger_initialised = false;
LogFileWriterData App::file_logger_data{};
Log::Logger App::logger{};

void App::init_translations() {
  if (!translations_initialised) {
    Log::LoadTranslation(&Log::messages[Log::LANG_EN],
                         "assets/translations/en_US.json");
    Log::LoadTranslation(&Log::messages[Log::LANG_PL],
                         "assets/translations/pl_PL.json");
    translations_initialised = true;
  }
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
  init_translations();

  if (!set_directory_globals())
    return false;

  if (!make_directories(FileUtils::APP_ROOT))
    return false;

  init_logger();

  if (!Renderer::Render::Init())
    return false;

  if (!render.init("Reaktory", {640, 480}))
    return false;

  render.userdata = this;
  render.SetKeyCallback([](Renderer::Render &window, int key, int scancode,
                           int action, int mods) {
    App *app = (App *)window.userdata;
    if (app->does_have_modal())
      return;

    if (action == GLFW_PRESS) {
      if (key == GLFW_KEY_O && mods & GLFW_MOD_CONTROL) {
        std::cerr << "CTRL+O\n";
      }
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

  new_scene_img =
      render.LoadImageFromMemory(scene_new_png_data, scene_new_png_size)
          .ok_unchecked();
  new_scene_tex = render.LoadTexture(new_scene_img).ok_unchecked();

  icon_tex = render.LoadTexture(icon).ok_unchecked();
  icons = render.LoadTexture(icons_, true).ok_unchecked();

  render.SetWindowIcon(icon);

  return true;
}

void App::shutdown() {
  free(new_scene_img.pixels);
  render.UnloadTexture(new_scene_tex);
  render.UnloadTexture(icon_tex);
  render.UnloadTexture(icons);

  render.cleanup();
  Renderer::Render::Cleanup();
}
#define ICONS_MODULO 4
#define ICONS_IDX_HEIGHT 4

struct imrect2 {
  ImVec2 s;
  ImVec2 e;
};

imrect2 icon_cords(int idx) {
  ImVec2 start =
      ImVec2((idx % ICONS_MODULO) * (1.0f / ICONS_MODULO),
             1.0f - ((idx / ICONS_MODULO) * (1.0f / ICONS_IDX_HEIGHT)));
  ImVec2 end =
      ImVec2(((idx % ICONS_MODULO) + 1) * (1.0f / ICONS_MODULO),
             1.0f - (((idx / ICONS_MODULO) + 1) * (1.0f / ICONS_IDX_HEIGHT)));
  imrect2 rect{start, end};
  return rect;
}

void App::AddIconToDrawlist(int idx, ImVec2 offset) {
  ImVec2 pos = ImGui::GetItemRectMin();
  ImDrawList *drawlist = ImGui::GetWindowDrawList();

  imrect2 ic = icon_cords(idx);
  drawlist->AddImage(
      (ImTextureRef)icons, ImVec2(pos.x + 1 + offset.x, pos.y + 1 + offset.y),
      ImVec2(pos.x + 21 + offset.x, pos.y + 21 + offset.y), ic.s, ic.e);
}

bool App::IconMenuItem(int idx, const char *label) {
  bool ret = ImGui::MenuItem(label);
  AddIconToDrawlist(idx);
  return ret;
}

bool CenteredButton(const char *label) {
  ImGui::SetCursorPosX(
      (ImGui::GetWindowSize().x - ImGui::CalcTextSize(label).x) / 2);
  return ImGui::Button(label);
}

void App::run() {
  render.makeContextCurrent();
  while (!render.WindowShouldClose()) {
    Renderer::Render::PullEvents();
    if (!render.IsMinimised()) {
      render.BeginFrame();
      ImGui::DockSpaceOverViewport();
      // ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
      //                              ImGuiDockNodeFlags_PassthruCentralNode);

      if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("Plik")) {
          ImGui::MenuItem(" " ICON_FA_FILE_ARROW_DOWN
                          "  (ctrl+S) Zapisz (wszystko)");
          IconMenuItem(0, "      (ctrl+O) Otwórz dostępną scenę");
          IconMenuItem(0, "      (ctrl+N) Utwórz nową scenę");
          IconMenuItem(9, "      (ctrl+I) Importuj scenę");
          IconMenuItem(10, "      (ctrl+E) Eksportuj scenę");
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edycja")) {
          IconMenuItem(5, "      (alt+D) Załaduj model");
          IconMenuItem(6, "      (alt+F) Załaduj teksturę");
          ImGui::Separator();

          bool use_local_cords = true;
          ImGui::Checkbox("      Operacje w przestrzeni lokalnej",
                          &use_local_cords);

          AddIconToDrawlist(2, ImVec2(24, 0));

          ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
      }

      ImGui::ShowDemoWindow();

      if (ImGui::Begin(ICON_FA_CUBES " Scena")) {
      }
      ImGui::End();

      if (ImGui::Begin(ICON_FA_WRENCH " Właściwości")) {
      }
      ImGui::End();

      if (ImGui::Begin(ICON_FA_DRAW_POLYGON " Modele")) {
      }
      ImGui::End();

      if (ImGui::Begin(ICON_FA_IMAGES " Tekstury")) {
      }
      ImGui::End();

      if (ImGui::Begin("Scena")) {
        if (scenes.size() <= 0) {
          ImVec2 wsize = ImGui::GetWindowSize();
          float min_axis = wsize.x < wsize.y ? wsize.x : wsize.y;
          float img_size = min_axis > 400 ? 250 : min_axis - 150;

          ImGui::SetCursorPos(
              ImVec2((wsize.x - img_size) / 2, (wsize.y - img_size) / 2));

          ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_WindowBg);
          if (ImGui::ImageButton("##Dodaj scenę", (ImTextureRef)new_scene_tex,
                                 ImVec2(img_size, img_size)))
            add_scene();
          if (CenteredButton("Brak otwartej sceny."))
            add_scene();
          if (CenteredButton("Otwórz albo utwórz jedną."))
            add_scene();
          ImGui::PopStyleColor(1);
        }
      }
      ImGui::End();

      render.EndFrame();
    }
  }
}

void App::add_scene() {
  scenes.push_back(Scene{});
}
