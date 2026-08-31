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

#ifndef NDEBUG
  if (ImGui::Begin("Debug")) {
    ImGui::Text("LMB: %1b", mousebuttonL);
    ImGui::Text("RMB: %1b", mousebuttonR);
    if (selected_scene_idx)
      ImGui::Text("Selected scene %s\n  %p", selected_scene_idx->c_str(),
                  selected_scene);
    else
      ImGui::Text("Selected scene (no name)\n  %p", selected_scene);
  }
  ImGui::End();
#endif

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
    scene_window_selected = ImGui::IsWindowFocused();
    if (scenes.size() <= 0) {
      ImVec2 wsize = ImGui::GetWindowSize();
      float min_axis = wsize.x < wsize.y ? wsize.x : wsize.y;
      float img_size = min_axis > 400 ? 200 : min_axis - 200;

      ImGui::SetCursorPos(
          ImVec2((wsize.x - img_size) / 2, (wsize.y - img_size) / 2));

      ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_WindowBg);
      if (ImGui::ImageButton("##Dodaj scenę", (ImTextureRef)new_scene_tex,
                             ImVec2(img_size, img_size), ImVec2(0, 1),
                             ImVec2(1, 0)))
        add_scene();
      if (CenteredButton("Brak otwartej sceny."))
        add_scene();
      if (CenteredButton("Stwórz scenę."))
        add_scene();
      if (CenteredButton("Otwórz scenę."))
        open_scene();
      ImGui::PopStyleColor(1);
    } else {
      scene_window_selected = ImGui::IsWindowFocused();
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

      ImGui::SetCursorPos(ImVec2(0, 25));
      if (ImGui::BeginTabBar("scene_tab")) {
        selected_scene_idx = nullptr;
        for (auto &[name, scene] : scenes) {
          if (ImGui::BeginTabItem(name.c_str())) {
            selected_scene_idx = &name;
            selected_scene = &scene;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 sregion = ImGui::GetContentRegionMax();

            sregion = ImVec2(sregion.x + 8, sregion.y - 39);

            ImVec2 imTL = ImVec2(pos.x - 8, pos.y - 4);
            ImVec2 imBR = ImVec2(pos.x + sregion.x - 8, pos.y + sregion.y - 4);

            Renderer::rect_size sp = render.GetCursorPosition();
            in_window_cursor_pos = Renderer::rect_size{
                sp.width - imTL.x, -1 * (sp.height - imBR.y)};

            scene.updateMousePos(in_window_cursor_pos);
            scene.updateMouseButtonState(mousebuttonL, mousebuttonR);
            scene.updateBodyMovement(movement_input);
            scene.resize({sregion.x, sregion.y});
            scene.render(render);
            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureRef)scene.screen_canvas, imTL, imBR, ImVec2(0, 1),
                ImVec2(1, 0));

            ImGui::EndTabItem();
          }
        }
        ImGui::EndTabBar();
      }

      if (!selected_scene_idx)
        selected_scene = nullptr;

      ImGui::PopStyleVar();
    }
  }
  ImGui::End();

  // [modals] -----
  // //
  if (current_modal)
    ImGui::OpenPopup(current_modal);

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  do {
    if (ImGui::BeginPopupModal("Nowa scena", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {

      ImGui::InputText("Nazwa sceny", name_buffer, 200);
      if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        memset(name_buffer, 0, 200);
        ImGui::CloseCurrentPopup();
        current_modal = nullptr;
      }

      if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        std::filesystem::path path =
            FileUtils::APP_ROOT / "scenes" / std::filesystem::path(name_buffer);

        if (!add_scene(name_buffer)) {
          ImGui::EndPopup();
          break;
        }
        memset(name_buffer, 0, 200);
        ImGui::CloseCurrentPopup();
        current_modal = nullptr;
      }
      ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Wybór sceny", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {

      if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        ImGui::CloseCurrentPopup();
        current_modal = nullptr;
      }

      if (ImGui::BeginCombo("Scena", owned_string.c_str())) {
        static ImGuiTextFilter filter;
        if (ImGui::IsWindowAppearing()) {
          ImGui::SetKeyboardFocusHere();
          filter.Clear();
        }

        filter.Draw("Wybierz scenę");

        for (const auto &ent : std::filesystem::directory_iterator(
                 FileUtils::APP_ROOT / "scenes")) {
          if (std::filesystem::is_directory(ent.path())) {
            std::string path_str = ent.path().string();
            if (filter.PassFilter(path_str.c_str())) {
              if (ImGui::Selectable(path_str.c_str(), false)) {
                owned_string = path_str;
              }
            }
          }
        }
        ImGui::EndCombo();
      }

      if (ImGui::Button("Otwórz")) {
        if (!open_scene(owned_string.substr(
                (FileUtils::APP_ROOT / "scenes").string().size() + 1))) {
          ImGui::EndPopup();
          break;
        }
        owned_string = std::string();
        ImGui::CloseCurrentPopup();
        current_modal = nullptr;
      }

      ImGui::EndPopup();
    }
  } while (0);
  // \\
  // [modals] -----

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
