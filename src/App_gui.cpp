#include "Renderer.hpp"
#include "imgui.h"
#include "imgui_internal.h"
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

void App::AddIconToDrawlist(int idx, ImVec2 offset, Renderer::rTexture2D tex) {
  ImVec2 pos = ImGui::GetItemRectMin();
  ImDrawList *drawlist = ImGui::GetWindowDrawList();

  imrect2 ic = icon_cords(idx);
  drawlist->AddImage((ImTextureRef)(tex ? tex : icons),
                     ImVec2(pos.x + 1 + offset.x, pos.y + 1 + offset.y),
                     ImVec2(pos.x + 21 + offset.x, pos.y + 21 + offset.y), ic.s,
                     ic.e);
}

bool App::IconMenuItem(int idx, const char *label, Renderer::rTexture2D tex) {
  bool ret = ImGui::MenuItem(label);
  AddIconToDrawlist(idx, ImVec2(0, 0), tex);
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
      if (IconMenuItem(0, "      (ctrl+O) Otwórz dostępną scenę"))
        open_scene();
      if (IconMenuItem(0, "      (ctrl+N) Utwórz nową scenę"))
        add_scene();
      IconMenuItem(9, "      (ctrl+I) Importuj scenę");
      IconMenuItem(10, "      (ctrl+E) Eksportuj scenę");
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edycja")) {
      IconMenuItem(5, "      (alt+D) Załaduj model");
      IconMenuItem(6, "      (alt+F) Załaduj teksturę");
      ImGui::Separator();

      bool use_local_cords = true;
      ImGui::Checkbox("      Osi względne do objektu", &use_local_cords);

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
          if (ImGui::BeginTabItem(name.c_str(), NULL,
                                  &scene == scene_to_be_selected
                                      ? ImGuiTabItemFlags_SetSelected
                                      : ImGuiTabItemFlags_None)) {

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
            ImDrawList *drawlist = ImGui::GetWindowDrawList();
            drawlist->AddImage((ImTextureRef)scene.screen_canvas, imTL, imBR,
                               ImVec2(0, 1), ImVec2(1, 0));

            ImGui::EndTabItem();

            ImVec2 kbwnd_size(120, 160);
            struct {
              int icon;
              const char *label;
            } keybinds_prev[] = {
                {0, "       zaznacz"},       {1, "       obrót kamerą"},
                {2, "       przód"},         {3, "       tył"},
                {4, "       lewo"},          {5, "       prawo"},
                {6, "       (spacja) góra"}, {7, "       (shift) dół"},
            };

            for (int i = 0;
                 i < sizeof(keybinds_prev) / sizeof(keybinds_prev[0]); i++) {
              ImVec2 offset =
                  ImVec2(pos.x + sregion.x - kbwnd_size.x,
                         pos.y + sregion.y - kbwnd_size.y + (18 * i));
              drawlist->AddText(offset, ImColor(255, 255, 255),
                                keybinds_prev[i].label);

              imrect2 ic = icon_cords(keybinds_prev[i].icon);
              drawlist->AddImage((ImTextureRef)keybinds_tex,
                                 ImVec2(1 + offset.x, 1 + offset.y),
                                 ImVec2(21 + offset.x, 21 + offset.y), ic.s,
                                 ic.e);
            }
          }
        }
        ImGui::EndTabBar();
      }

      ImGui::PopStyleVar();
    }
  }
  ImGui::End();

  scene_to_be_selected = nullptr;
  if (ImGui::Begin(ICON_FA_CUBES " Scena")) {
    ImGui::Text(ICON_FA_CUBES " Sceny (%d)", scenes.size());
    ImVec2 wsize = ImGui::GetWindowSize();
    ImGui::BeginChild("scena_child", ImVec2(wsize.x - 16, 70));
    for (auto &[name, scene] : scenes) {
      if (ImGui::Selectable(name.c_str(), &scene == selected_scene)) {
        selected_scene_idx = &name;
        selected_scene = &scene;
        scene_to_be_selected = &scene;
      }
    }
    ImGui::EndChild();

    ImGui::Text(ICON_FA_CUBE " Objekty (0)");
  }
  ImGui::End();

  if (!selected_scene_idx)
    selected_scene = nullptr;

  if (ImGui::Begin(ICON_FA_WRENCH " Właściwości")) {
  }
  ImGui::End();

  if (ImGui::Begin(ICON_FA_DRAW_POLYGON " Modele")) {
    if (selected_scene) {
      ImGui::Text(ICON_FA_DRAW_POLYGON " Modele (%d)",
                  selected_scene->modelManager.GetModelsMap().size());

      for (const auto &[name, model] :
           selected_scene->modelManager.GetModelsMap()) {

        ImGui::Selectable(name.c_str(), false);
      }
    }
  }
  ImGui::End();

  if (ImGui::Begin(ICON_FA_IMAGES " Tekstury")) {
  }
  ImGui::End();
}
