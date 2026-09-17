#include "FileUtils.hpp"
#include "ObjectPool.hpp"
#include "Renderer.hpp"
#include "imgui.h"
#include "pfd/pfd.hpp"
#include <App.hpp>
#include <FontsAwesome/IconsFontAwesome6.h>
#include <cstdio>
#include <iostream>

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

void App::draw_object_tree(Object *node, int idx) {
  ImGui::PushID(idx);
  ImDrawList *drawlist = ImGui::GetWindowDrawList();

  if (node->children.size() == 0) {
    ImGui::Dummy(ImVec2(16 + 1, 18));
    ImVec2 bpos = ImGui::GetItemRectMin();

    // drawlist->AddCircleFilled(ImVec2(bpos.x + 8, bpos.y + 9), 4,
    //                           ImColor(100, 100, 100));
    drawlist->AddRectFilled(ImVec2(bpos.x + 7, bpos.y),
                            ImVec2(bpos.x + 9, bpos.y + 18),
                            ImColor(50, 50, 50));
    drawlist->AddRectFilled(ImVec2(bpos.x + 7, bpos.y + 8),
                            ImVec2(bpos.x + 15, bpos.y + 10),
                            ImColor(50, 50, 50));

  } else {
    if (ImGui::Button("##collapse_button", ImVec2(16 + 1, 18)))
      node->collapsed = !node->collapsed;
    ImVec2 bpos = ImGui::GetItemRectMin();
    ImVec2 bpos_max = ImGui::GetItemRectMax();
    drawlist->AddRectFilled(bpos, bpos_max,
                            ImGui::GetColorU32(ImGuiCol_WindowBg));
    if (node->collapsed)
      drawlist->AddTriangleFilled(
          ImVec2(bpos.x + 4, bpos.y + 5), ImVec2(bpos.x + 4, bpos.y + 13),
          ImVec2(bpos.x + 12, bpos.y + 9), ImColor(ImColor(100, 100, 100)));
    else
      drawlist->AddTriangleFilled(
          ImVec2(bpos.x + 4, bpos.y + 5), ImVec2(bpos.x + 8, bpos.y + 13),
          ImVec2(bpos.x + 12, bpos.y + 5), ImColor(ImColor(100, 100, 100)));
  }

  ImGui::SameLine();
  if (ImGui::Button("##baton", ImVec2(ImGui::GetContentRegionAvail().x, 18)))
    selected_scene->selected_object = node->toHandle(idx);
  ImVec2 bpos2 = ImGui::GetItemRectMin();
  ImVec2 bpos2_max = ImGui::GetItemRectMax();

  if (node->toHandle(idx) != selected_scene->selected_object)
    drawlist->AddRectFilled(bpos2, bpos2_max,
                            ImGui::GetColorU32(ImGuiCol_WindowBg));

  if (const oNode *onode = std::get_if<oNode>(&node->variant)) {
    float char_vis_width = ImGui::CalcTextSize(ICON_FA_CUBE).x;
    drawlist->AddText(ImVec2(bpos2.x + 11 - (char_vis_width / 2), bpos2.y + 1),
                      ImColor(50, 100, 200), ICON_FA_CUBE);
  } else if (const oModel *omodel = std::get_if<oModel>(&node->variant)) {
    float char_vis_width = ImGui::CalcTextSize(ICON_FA_DRAW_POLYGON).x;
    drawlist->AddText(ImVec2(bpos2.x + 11 - (char_vis_width / 2), bpos2.y + 1),
                      ImColor(200, 50, 25), ICON_FA_DRAW_POLYGON);
  } else if (const oFormula *oformula = std::get_if<oFormula>(&node->variant)) {
    float char_vis_width = ImGui::CalcTextSize(ICON_FA_SQUARE_ROOT_VARIABLE).x;
    drawlist->AddText(ImVec2(bpos2.x + 11 - (char_vis_width / 2), bpos2.y + 1),
                      ImColor(25, 200, 50), ICON_FA_SQUARE_ROOT_VARIABLE);
  }
  drawlist->AddText(ImVec2(bpos2.x + 22 + 3, bpos2.y + 1),
                    ImColor(220, 220, 220), node->name.c_str());

  ImGui::Indent(8);
  for (int16_t i = 0; i < node->children.size() && !node->collapsed; i++)
    draw_object_tree(selected_scene->objPool.getChildOf(node, i),
                     node->children[i]);
  ImGui::Unindent(8);
  ImGui::PopID();
}

void App::draw_gui() {
  // static char model_name_buffer[200] = {0};
  // static char texture_name_buffer[200] = {0};
  // static std::string owned_string;

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
            ImVec2 posi = ImVec2(pos.x + sregion.x - kbwnd_size.x,
                                 pos.y + sregion.y - kbwnd_size.y);

            drawlist->AddRectFilled(
                posi, ImVec2(pos.x + sregion.x, pos.y + sregion.y),
                ImColor(0, 0, 0, 128));

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
              ImVec2 offset = ImVec2(posi.x, posi.y + (18 * i));
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
  if (ImGui::Begin(ICON_FA_CUBES " Scena") && selected_scene) {
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

    ImGui::Text(ICON_FA_CUBE " Objekty (%d)", selected_scene->objPool.size());
    ImGui::SameLine();
    if (ImGui::BeginCombo("##dodaj_objekt", "dodaj objekt")) {
      if (ImGui::Selectable("model", false)) {
        selected_scene->objPool.add_model_node(
            selected_scene->resMan, selected_scene->selected_object, "");
      }
      ImGui::EndCombo();
    }

    Object *root = selected_scene->objPool.get(selected_scene->objPool.rootH());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    draw_object_tree(root, 0);
    ImGui::PopStyleVar(1);
  };
  ImGui::End();

  if (!selected_scene_idx)
    selected_scene = nullptr;

  if (ImGui::Begin(ICON_FA_WRENCH " Właściwości")) {
  }
  ImGui::End();

  if (ImGui::Begin(ICON_FA_DRAW_POLYGON " Modele") && selected_scene) {

    ImGui::Text(ICON_FA_DRAW_POLYGON " Modele (%d)",
                selected_scene->resMan.GetModelsMap().size());
    ImGui::SameLine();

    if (ImGui::Button(" + ")) {
      std::vector<std::string> models_to_load =
          ipfd::open_file("Wybór modeli", "",
                          {"Plik modelu (obj/m3d/fbx)", "*.obj;*.m3d;*.fbx",
                           "Wszystkie pliki", "*"},
                          ipfd::opt::multiselect);

      for (const auto &model : models_to_load) {
        selected_scene->resMan.ImportModel(
            FileUtils::RealFs{std::filesystem::path{}},
            std::filesystem::path{model});
      }
    }
    ImGui::SameLine();

    static ImGuiTextFilter filter;
    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
      filter.Clear();
    }

    ImGui::TextUnformatted(ICON_FA_MAGNIFYING_GLASS);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    filter.Draw("##Wybierz model");

    ImGui::BeginChild("###modele_child");
    for (const auto &[name, model] : selected_scene->resMan.GetModelsMap()) {
      if (filter.PassFilter(name.c_str()))
        ImGui::Selectable(name.c_str(), false);
    }
    ImGui::EndChild();
  }
  ImGui::End();

  if (ImGui::Begin(ICON_FA_IMAGES " Tekstury") && selected_scene) {

    ImGui::Text(ICON_FA_IMAGE " Tekstury (%d)",
                selected_scene->resMan.GetTexturesMap().size());
    ImGui::SameLine();

    if (ImGui::Button(" + ")) {
      std::vector<std::string> textures_to_load =
          ipfd::open_file("Wybór obrazka", "",
                          {"Plik obrazka (png/jp(e)g/gif)",
                           "*.png;*.jpg;*.jpeg;*.gif", "Wszystkie pliki", "*"},
                          ipfd::opt::multiselect);

      for (const auto &texture : textures_to_load) {
        selected_scene->resMan.ImportTexture(
            FileUtils::RealFs{std::filesystem::path{}},
            std::filesystem::path{texture});
      }
    }
    ImGui::SameLine();

    static ImGuiTextFilter filter;
    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
      filter.Clear();
    }

    ImGui::TextUnformatted(ICON_FA_MAGNIFYING_GLASS);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    filter.Draw("##Wybierz obraz");

    ImGui::BeginChild("###textury_child");
    float w_width = ImGui::GetWindowSize().x;
    float width_accumulator = 0;
    ImVec2 image_size = ImVec2(50, 50);
    for (const auto &[name, texture] :
         selected_scene->resMan.GetTexturesMap()) {

      if (!filter.PassFilter(name.c_str()))
        continue;

      ImGui::ImageButton(name.c_str(), (ImTextureRef)texture, image_size,
                         ImVec2(0, 1), ImVec2(1, 0));

      if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
        ImGui::SetTooltip(name.c_str());

      width_accumulator += image_size.x + 10;
      if (width_accumulator + image_size.x + 10 < w_width - 8)
        ImGui::SameLine();
      else
        width_accumulator = 0;
    }
    ImGui::EndChild();
  }
  ImGui::End();
}
