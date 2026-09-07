#include "imgui.h"
#include <App.hpp>
#include <FontsAwesome/IconsFontAwesome6.h>

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

// -------------------------------------------------------------------------------------------------------------------

void App::draw_gui() {
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
    ImGui::Text("Sceny (%d)", scenes.size());
    for (auto &[name, scene] : scenes) {
      if (ImGui::Selectable(name.c_str(), &scene == selected_scene)) {
        selected_scene_idx = &name;
        selected_scene = &scene;
      }
    }
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
                (int)(sp.width - imTL.x), (int)(-1 * (sp.height - imBR.y))};

            scene.updateMousePos(in_window_cursor_pos);
            scene.updateMouseButtonState(mousebuttonL, mousebuttonR);
            scene.updateBodyMovement(movement_input);
            scene.resize({(int)sregion.x, (int)sregion.y});
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
}
