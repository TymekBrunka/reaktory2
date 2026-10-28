#include <App.hpp>

#include "FileUtils.hpp"
#include "Model.hpp"
#include "ObjectPool.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "imgui.h"
#include "pfd/pfd.hpp"
#include <FontsAwesome/IconsFontAwesome6.h>
#include <cfloat>
#include <cstdio>
#include <imgui_stdlib.h>
#include <iostream>

#include <ImGuizmo.h>

#define ICONS_MODULO 4
#define ICONS_IDX_HEIGHT 4

struct imrect2 {
  ImVec2 s;
  ImVec2 e;
};

static imrect2 icon_cords(int idx) {
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

static bool CenteredButton(const char *label) {
  ImGui::SetCursorPosX(
      (ImGui::GetWindowSize().x - ImGui::CalcTextSize(label).x) / 2);
  return ImGui::Button(label);
}

void InputAllignedText(const char *text, float height = 22) {
  int textw = ImGui::CalcTextSize(text).x;
  ImGui::Dummy(ImVec2(textw, height));
  ImVec2 pmin = ImGui::GetItemRectMin();
  ImGui::GetWindowDrawList()->AddText(
      ImVec2(pmin.x, pmin.y - 11 + (height / 2) + 2), ImColor(255, 255, 255),
      text);
}

void OpenCustomCombo(const char *name) { ImGui::OpenPopup(name); }

bool BeginCustomCombo(const char *label, ImVec2 *wpos, ImVec2 *wpos2) {
  *wpos = ImGui::GetItemRectMax();
  *wpos2 = ImGui::GetItemRectMin();
  return ImGui::BeginPopup(label, ImGuiWindowFlags_NoMove);
}

void EndCustomCombo(ImVec2 wpos, ImVec2 wpos2) {
  ImVec2 wsize = ImGui::GetWindowSize();
  ImVec2 final;
  if (wpos.x - wpos2.x < wsize.x)
    final = ImVec2(wpos.x - wsize.x, wpos.y);
  else
    final = ImVec2(wpos2.x, wpos.y);

  if (final.x < 0)
    final.x = wpos2.x;

  if (final.x < 0)
    final.x = 0;
  else if (final.x + wsize.x > ImGui::GetIO().DisplaySize.x)
    final.x = ImGui::GetIO().DisplaySize.x - wsize.x;

  if (final.y + wsize.y > ImGui::GetIO().DisplaySize.y)
    final.y = ImGui::GetIO().DisplaySize.y - wsize.y;

  ImGui::SetWindowPos(final);
  ImGui::EndPopup();
}

bool BeginCustomCombo2(ImVec2 *wpos, ImVec2 *wpos2, const char *label,
                       const char *icon, const char *preview, int idx = 1,
                       float width = -1) {

  ImDrawList *drawlist = ImGui::GetWindowDrawList();
  ImGui::PushID(idx);
  bool ret = ImGui::Button(
      "##skrypt",
      ImVec2(width == -1 ? ImGui::GetContentRegionAvail().x - 8 : width, 26));
  ImGui::PopID();
  ImVec2 bpos = ImGui::GetItemRectMin();
  ImVec2 bpos2 = ImGui::GetItemRectMax();
  drawlist->AddRectFilled(bpos, ImVec2(bpos.x + 16, bpos2.y), 0xFF54371E);
  drawlist->AddTriangleFilled(
      ImVec2(bpos.x + 4, bpos.y + 8), ImVec2(bpos.x + 12, bpos.y + 8),
      ImVec2(bpos.x + 8, bpos.y + 16), ImColor(255, 255, 255));

  // float textw = ImGui::CalcTextSize(text).x;
  drawlist->AddText(ImVec2(bpos.x + 20, bpos.y + 6), ImColor(255, 255, 255),
                    icon);

  drawlist->AddText(ImVec2(bpos.x + 40, bpos.y + 5), ImColor(255, 255, 255),
                    preview);

  if (ret)
    OpenCustomCombo(label);
  return BeginCustomCombo(label, wpos, wpos2);
}

bool BeginSection(int idx, bool *collapse, const char *label, ImU32 hdrColor,
                  ImU32 bgColor) {
  ImDrawList *drawlist = ImGui::GetWindowDrawList();

  int textw = ImGui::CalcTextSize(label).x;
  ImGui::PushID(idx);
  if (ImGui::Button("##właściwości_btn", ImVec2(22, 22)))
    *collapse = !*collapse;
  ImGui::PopID();
  ImVec2 pmin = ImGui::GetItemRectMin();

  drawlist->AddRectFilled(
      pmin, ImVec2(pmin.x + ImGui::GetContentRegionMax().x - 8, pmin.y + 22),
      hdrColor);

  if (*collapse)
    drawlist->AddTriangleFilled(
        ImVec2(pmin.x + 4, pmin.y + 4), ImVec2(pmin.x + 18, pmin.y + 11),
        ImVec2(pmin.x + 4, pmin.y + 18), ImColor(255, 255, 255));
  else
    drawlist->AddTriangleFilled(
        ImVec2(pmin.x + 4, pmin.y + 4), ImVec2(pmin.x + 18, pmin.y + 4),
        ImVec2(pmin.x + 11, pmin.y + 18), ImColor(255, 255, 255));

  drawlist->AddText(ImVec2(pmin.x + 30, pmin.y + 2), ImColor(255, 255, 255),
                    label);

  ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
  return !*collapse &&
         ImGui::BeginChild(idx, ImVec2(ImGui::GetContentRegionAvail().x, 0),
                           ImGuiChildFlags_AlwaysAutoResize |
                               ImGuiChildFlags_AutoResizeY);
}

// AAAAAA
// -----------------------------------------------------------------------------------------------------------

static void type_object_draw(Scene *selected_scene, ImVec2 *wpos, ImVec2 *wpos2,
                             Object *node, int idx) {
  if (BeginCustomCombo2(&wpos, &wpos2, "model##model", ICON_FA_DRAW_POLYGON,
                        node->model.c_str())) {

    static ImGuiTextFilter filter;
    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
      filter.Clear();
    }
    filter.Draw("model##model");

    for (const auto &[name, model] : selected_scene->resMan.GetModelsMap()) {

      if (!filter.PassFilter(name.c_str()))
        continue;

      if (ImGui::Selectable(name.c_str(), node->toHandle(idx) == name)) {
      }
    }
    EndCustomCombo(wpos, wpos2);
  }

  static void type_omodel_draw(Scene * selected_scene, ImVec2 * wpos,
                               ImVec2 * wpos2, oModel * omodel) {
    ImGui::SetCursorPosX(8);
    InputAllignedText("Model", 26);
    ImGui::SameLine();

    if (BeginCustomCombo2(wpos, wpos2, "model", ICON_FA_DRAW_POLYGON,
                          omodel->model.c_str())) {

      static ImGuiTextFilter filter;
      if (ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
        filter.Clear();
      }
      filter.Draw("model");

      for (const auto &[name, model] : selected_scene->resMan.GetModelsMap()) {

        if (!filter.PassFilter(name.c_str()))
          continue;

        if (ImGui::Selectable(name.c_str(), omodel->model == name)) {
          omodel->model = name;
          omodel->animation_idx = -1;
          size_t old_size = omodel->materials.size();
          omodel->materials.resize(model.materials.size());
          for (int i = old_size; i < omodel->materials.size(); i++) {
            omodel->materials[i] = Material{};
          }
        }
      }
      EndCustomCombo(*wpos, *wpos2);
    }
  }

  static void type_material_draw(Scene * selected_scene, ImVec2 * wpos,
                                 ImVec2 * wpos2, Material & material) {

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
    ImGui::ColorEdit4("##kolor", &material.color_diffuse.x,
                      ImGuiColorEditFlags_NoInputs |
                          ImGuiColorEditFlags_NoLabel);
    ImGui::SameLine();

    if (BeginCustomCombo2(wpos, wpos2, "##obraz", ICON_FA_IMAGES,
                          material.diffuse1.c_str(), 1,
                          ImGui::GetContentRegionAvail().x - 64)) {
      static ImGuiTextFilter filter;
      if (ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
        filter.Clear();
      }
      filter.Draw("obrazy");

      float w_width = ImGui::GetWindowSize().x;
      ImVec2 image_size = ImVec2(50, 50);
      float width_accumulator = image_size.x + 10;

      {
        if (ImGui::Button("(brak)",
                          ImVec2(image_size.x + 6, image_size.y + 6))) {
          material.diffuse1 = "";
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
          ImGui::SetTooltip("(brak)");

        ImGui::SameLine();
      }

      for (const auto &[name, texture] :
           selected_scene->resMan.GetTexturesMap()) {

        if (!filter.PassFilter(name.c_str()))
          continue;

        if (ImGui::ImageButton(name.c_str(), (ImTextureRef)texture, image_size,
                               ImVec2(0, 1), ImVec2(1, 0))) {

          material.diffuse1 = name;
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
          ImGui::SetTooltip(name.c_str());

        width_accumulator += image_size.x + 10;
        if (width_accumulator + image_size.x + 10 < w_width - 8)
          ImGui::SameLine();
        else
          width_accumulator = 0;
      }

      EndCustomCombo(*wpos, *wpos2);
    }
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 15);

    int diffuse1 = selected_scene->resMan.GetTexture(material.diffuse1);

    if (diffuse1 != -1) {
      ImGui::Image((ImTextureRef)diffuse1, ImVec2(50, 50), ImVec2(0, 1),
                   ImVec2(1, 0));
    } else {
      ImGui::Button("(brak)", ImVec2(50, 50));
    }
  }

  // -------------------------------------------------------------------------------------------------------------------

  void App::draw_object_tree(Object * node, int idx, bool is_descendant) {
    ImGui::PushID(idx);
    ImDrawList *drawlist = ImGui::GetWindowDrawList();
    static bool is_dragging = false;
    static objH object_to_drop = objH{.gen = 0, .idx = -1};

    ImVec2 wsize = ImGui::GetWindowSize();
    ImVec2 cpos = ImGui::GetMousePos();
    cpos = ImVec2(cpos.x - ImGui::GetWindowPos().x,
                  cpos.y - ImGui::GetWindowPos().y + 20);

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      is_dragging = false;
    }

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
    if (ImGui::Button("##baton",
                      ImVec2(ImGui::GetContentRegionAvail().x, 18))) {
      selected_scene->selected_object = node->toHandle(idx);
    }

    if (ImGui::IsItemActive()) {
      is_dragging = true;
      object_to_drop = node->toHandle(idx);
    }

    ImVec2 bpos2 = ImGui::GetItemRectMin();
    ImVec2 bpos2_max = ImGui::GetItemRectMax();

    if (node->toHandle(idx) != selected_scene->selected_object)
      drawlist->AddRectFilled(bpos2, bpos2_max,
                              ImGui::GetColorU32(ImGuiCol_WindowBg));

    if (cpos.x <= wsize.x && cpos.x >= 0 && cpos.y <= wsize.y && cpos.y >= 0)
      if (!is_descendant && node->toHandle(idx) != object_to_drop &&
          cpos.y < bpos2_max.y && cpos.y > bpos2.y)

        if (is_dragging)
          drawlist->AddRectFilled(ImVec2(bpos2.x, bpos2_max.y),
                                  ImVec2(bpos2_max.x, bpos2_max.y - 2),
                                  ImColor(0, 100, 200));

        else if (Object *obj = selected_scene->objPool.get(object_to_drop)) {

          if (Object *parent = selected_scene->objPool.get(obj->parent)) {
            for (int16_t i = 0; i < parent->children.size(); i++) {
              if (parent->children[i] == object_to_drop.idx) {
                parent->children.erase(parent->children.begin() + i);
                break;
              }
            }
          }

          node->children.push_back(object_to_drop.idx);
          obj->parent = node->toHandle(idx);
          object_to_drop.idx = -1;
          ImGui::PopID();
          return;
        }

    if (const oNode *onode = std::get_if<oNode>(&node->variant)) {
      float char_vis_width = ImGui::CalcTextSize(ICON_FA_CUBE).x;
      drawlist->AddText(
          ImVec2(bpos2.x + 11 - (char_vis_width / 2), bpos2.y + 1),
          ImColor(50, 100, 200), ICON_FA_CUBE);
    } else if (const oModel *omodel = std::get_if<oModel>(&node->variant)) {
      float char_vis_width = ImGui::CalcTextSize(ICON_FA_DRAW_POLYGON).x;
      drawlist->AddText(
          ImVec2(bpos2.x + 11 - (char_vis_width / 2), bpos2.y + 1),
          ImColor(200, 50, 25), ICON_FA_DRAW_POLYGON);
    }
    drawlist->AddText(ImVec2(bpos2.x + 22 + 3, bpos2.y + 1),
                      ImColor(220, 220, 220), node->name.c_str());

    ImGui::Indent(8);
    for (int16_t i = 0; i < node->children.size() && !node->collapsed; i++)
      draw_object_tree(selected_scene->objPool.getChildOf(node, i),
                       node->children[i],
                       node->toHandle(idx) == object_to_drop || is_descendant);
    ImGui::Unindent(8);
    ImGui::PopID();

    object_to_drop.idx = idx == 0 && !is_dragging ? -1 : object_to_drop.idx;
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
      if (selected_scene_idx) {
        ImGui::Text("Selected scene %s\n  %p", selected_scene_idx->c_str(),
                    selected_scene);

        ImGui::TextUnformatted("objPool");
        Object *root =
            selected_scene->objPool.get(selected_scene->objPool.rootH());
        for (int16_t i = 0; i < selected_scene->objPool.real_size(); i++) {
          Object *obj = root + i;
          if (std::get_if<oEmptySlot>(&obj->variant))
            ImGui::Text("idx: %d (free slot)", i);
          else {
            ImGui::Text("idx: %d, parent: %d:%d, name: %s", i, obj->parent.idx,
                        obj->parent.gen, obj->name.c_str());
          }
        }
      } else
        ImGui::Text("Selected scene (none)\n  %p", selected_scene);
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
              ImVec2 imBR =
                  ImVec2(pos.x + sregion.x - 8, pos.y + sregion.y - 4);

              Renderer::rect_size sp = render.GetCursorPosition();
              in_window_cursor_pos = Renderer::rect_size{
                  (int)(sp.width - imTL.x), (int)(-1 * (sp.height - imBR.y))};

              ImDrawList *drawlist = ImGui::GetWindowDrawList();

              ImGuizmo::SetDrawlist(drawlist);
              ImGuizmo::SetRect(imTL.x, imTL.y, imBR.x - imTL.x,
                                imBR.y - imTL.y);

              // moved this up bc otherwise the guizmo would be covered by scene
              // canvas
              drawlist->AddImage((ImTextureRef)scene.screen_canvas, imTL, imBR,
                                 ImVec2(0, 1), ImVec2(1, 0));

              scene.updateMousePos(in_window_cursor_pos);
              scene.updateMouseButtonState(mousebuttonL, mousebuttonR);
              scene.updateBodyMovement(movement_input);
              scene.resize({(int)sregion.x, (int)sregion.y});
              scene.render(render);
              // drawlist->AddImage((ImTextureRef)scene.screen_canvas, imTL,
              // imBR,
              //                    ImVec2(0, 1), ImVec2(1, 0));

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

      Object *root =
          selected_scene->objPool.get(selected_scene->objPool.rootH());
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
      draw_object_tree(root, 0);
      ImGui::PopStyleVar(1);
    };
    ImGui::End();

    if (!selected_scene_idx)
      selected_scene = nullptr;

    if (ImGui::Begin(ICON_FA_WRENCH " Właściwości") && selected_scene) {
      Object *node =
          selected_scene->objPool.get(selected_scene->selected_object);

      if (node) {
        static bool hideobjprop = false;

        if (BeginSection(1, &hideobjprop, ICON_FA_CUBE " objekt",
                         ImColor(50, 50, 50), ImColor(25, 25, 25))) {
          ImDrawList *drawlist = ImGui::GetWindowDrawList();
          ImGui::Dummy(ImVec2(2, 2));

          ImGui::SetCursorPosX(8);
          InputAllignedText("Nazwa");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 8);
          ImGui::InputText("##nazwa", &node->name);

          ImGui::SetCursorPosX(8);
          InputAllignedText("Skrypt", 26);
          ImGui::SameLine();

          ImVec2 wpos;
          ImVec2 wpos2;
          if (BeginCustomCombo2(
                  &wpos, &wpos2, "skrypt", ICON_FA_MICROCHIP,
                  node->script_class_idx == -1
                      ? "(brak)"
                      : selected_scene->objPool
                            .get_script_classes()[node->script_class_idx]
                            .name.c_str())) {
            ImDrawList *drawlist = ImGui::GetWindowDrawList();
            ImVec2 ra = ImGui::GetContentRegionAvail();

            static ImGuiTextFilter sc_filter;
            if (ImGui::IsWindowAppearing()) {
              ImGui::SetKeyboardFocusHere();
              sc_filter.Clear();
            }
            sc_filter.Draw("skrypt");
            ImGui::Dummy(ImVec2(0, 3));

            bool ret =
                ImGui::Selectable("##sc_button", -1 == node->script_class_idx,
                                  0, ImVec2(ra.x, 16));
            ImVec2 bpos = ImGui::GetItemRectMin();
            drawlist->AddText(ImVec2(bpos.x + 1, bpos.y + 2),
                              ImColor(255, 255, 255), "(brak)");
            if (ret)
              node->change_script_class(&selected_scene->objPool, -1);

            int sc_idx = 0;
            for (const auto &sc :
                 selected_scene->objPool.get_script_classes()) {

              if (!sc_filter.PassFilter(sc.name.c_str()))
                continue;

              ImGui::PushID(sc_idx);
              ret = ImGui::Selectable("##sc_button",
                                      sc_idx == node->script_class_idx, 0,
                                      ImVec2(ra.x, 16));

              bpos = ImGui::GetItemRectMin();
              drawlist->AddText(ImVec2(bpos.x + 2, bpos.y + 3),
                                ImColor(255, 255, 255), ICON_FA_MICROCHIP);
              drawlist->AddText(ImVec2(bpos.x + 22, bpos.y + 2),
                                ImColor(255, 255, 255), sc.name.c_str());
              if (ret)
                node->change_script_class(&selected_scene->objPool, sc_idx);

              ImGui::PopID();
              sc_idx++;
            }
            EndCustomCombo(wpos, wpos2);
          }
          ImGui::Dummy(ImVec2(2, 2));
        }
        if (!hideobjprop)
          ImGui::EndChild();
        ImGui::PopStyleColor(1);

        static bool hidescriptprop = false;

        if (node->script_class_idx != -1 &&
            BeginSection(2, &hidescriptprop, ICON_FA_MICROCHIP " skrypt",
                         ImColor(25, 150, 25), ImColor(0, 75, 25))) {
          ImGui::Dummy(ImVec2(2, 2));
          ImVec2 wpos;
          ImVec2 wpos2;

          const ScriptClass &sc =
              selected_scene->objPool
                  .get_script_classes()[node->script_class_idx];

          int i = 1;
          for (auto &param_ : node->lua_params) {
            ImGui::PushID(i);
            const ScriptClass::Param &scparam = sc.params[i - 1];
            LuaParam &param = *(LuaParam *)&param_;

            ImGui::SetCursorPosX(8);
            InputAllignedText(scparam.name.c_str(), 26);
            ImGui::SameLine();

            ImDrawList *drawlist = ImGui::GetWindowDrawList();
            ImVec2 cpos = ImGui::GetCursorPos();
            // bool ret = ImGui::Button(
            //     "##skrypt_", ImVec2(ImGui::GetContentRegionAvail().x - 8,
            //     26));
            bool ret = ImGui::Button("##skrypt_", ImVec2(16, 22));
            ImVec2 bpos = ImGui::GetItemRectMin();

            if (Nil *value = (Nil *)std::get_if<Nil>(&param)) {
              ImGui::SetCursorPos(ImVec2(cpos.x + 16, cpos.y));
              ret |= ImGui::Button(
                  "nil##nil", ImVec2(ImGui::GetContentRegionAvail().x - 8, 22));

            } else if (bool *value = (bool *)std::get_if<bool>(&param)) {
              ImGui::SetCursorPos(ImVec2(cpos.x + 16, cpos.y));
              ImGui::Checkbox("##bool##bool", value);

            } else if (int *value = (int *)std::get_if<int>(&param)) {
              ImGui::SetCursorPos(ImVec2(cpos.x + 16, cpos.y));
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 8);
              ImGui::InputInt("##int##int", value);

            } else if (float *value = (float *)std::get_if<float>(&param)) {
              ImGui::SetCursorPos(ImVec2(cpos.x + 16, cpos.y));
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 8);
              ImGui::InputFloat("##float##float", value);

            } else if (std::string *value =
                           (std::string *)std::get_if<std::string>(&param)) {
              ImGui::SetCursorPos(ImVec2(cpos.x + 16, cpos.y));
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 8);
              ImGui::InputText("##string##string", value);

            } else if (Material *value =
                           (Material *)std::get_if<Material>(&param)) {
              ImGui::SetCursorPos(ImVec2(cpos.x + 16, cpos.y));
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 8);
              type_material_draw(selected_scene, &wpos, &wpos2, *value);

            } else if (objH *value = (objH *)std::get_if<objH>(&param)) {
              ImGui::SetCursorPos(ImVec2(cpos.x + 16, cpos.y));
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 8);
              type_material_draw(selected_scene, &wpos, &wpos2, *value);

            } else if (_ModelName *value =
                           (_ModelName *)std::get_if<_ModelName>(&param)) {
              ImGui::SetCursorPos(ImVec2(cpos.x + 16, cpos.y));
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 8);

              if (BeginCustomCombo2(&wpos, &wpos2, "model##model",
                                    ICON_FA_DRAW_POLYGON,
                                    value->model.c_str())) {

                static ImGuiTextFilter filter;
                if (ImGui::IsWindowAppearing()) {
                  ImGui::SetKeyboardFocusHere();
                  filter.Clear();
                }
                filter.Draw("model##model");

                for (const auto &[name, model] :
                     selected_scene->resMan.GetModelsMap()) {

                  if (!filter.PassFilter(name.c_str()))
                    continue;

                  if (ImGui::Selectable(name.c_str(), value->model == name)) {
                    value->model = name;
                  }
                }
                EndCustomCombo(wpos, wpos2);
              }
            }

            ImVec2 bpos2 = ImGui::GetItemRectMax();
            drawlist->AddRectFilled(bpos, ImVec2(bpos.x + 16, bpos2.y),
                                    0xFF54371E);
            drawlist->AddTriangleFilled(
                ImVec2(bpos.x + 4, bpos.y + 8), ImVec2(bpos.x + 12, bpos.y + 8),
                ImVec2(bpos.x + 8, bpos.y + 16), ImColor(255, 255, 255));

            if (ret)
              OpenCustomCombo("skrypt");

            if (BeginCustomCombo("skrypt", &wpos, &wpos2)) {
              if (1 & scparam.types) {
                if (ImGui::Selectable("wartość logiczna"))
                  param = false;
              }
              if (2 & scparam.types) {
                if (ImGui::Selectable("liczba całkowita"))
                  param = 0;
              }
              if (4 & scparam.types) {
                if (ImGui::Selectable("liczba rzeczywista"))
                  param = 0.0f;
              }
              if (8 & scparam.types) {
                if (ImGui::Selectable("łańcuch znaku"))
                  param = std::string();
              }
              if (16 & scparam.types) {
                if (ImGui::Selectable("materiał"))
                  param = Material{};
              }
              if (32 & scparam.types) {
                if (ImGui::Selectable("objekt"))
                  param = objH{};
              }
              if (64 & scparam.types) {
                if (ImGui::Selectable("model"))
                  param = _ModelName{};
              }
              EndCustomCombo(wpos, wpos2);
            }

            ImGui::PopID();
            i++;
          }

          ImGui::Dummy(ImVec2(2, 2));
        }
        if (node->script_class_idx != -1 && !hidescriptprop)
          ImGui::EndChild();
        if (node->script_class_idx != -1)
          ImGui::PopStyleColor(1);

        if (oModel *omodel = (oModel *)std::get_if<oModel>(&node->variant)) {
          static bool hidemodelprop = false;
          if (BeginSection(3, &hidemodelprop, ICON_FA_DRAW_POLYGON " model",
                           0xFF0054DB, 0xFF0C1F3D)) {

            ImGui::Dummy(ImVec2(2, 2));
            ImVec2 wpos;
            ImVec2 wpos2;

            type_omodel_draw(selected_scene, &wpos, &wpos2, omodel);

            int i = 0;
            ImGui::SetCursorPosX(8);
            for (auto &material : omodel->materials) {
              ImGui::PushID(i);

              ImGui::SetCursorPosX(8);
              ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);

              InputAllignedText("Kolor", 26);
              ImGui::SameLine();
              ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 15);

              type_material_draw(selected_scene, &wpos, &wpos2, material);
              ImGui::PopID();
              i++;
            }
            ImGui::Dummy(ImVec2(0, 10));

            ManagedModel *model =
                selected_scene->resMan.GetModel(omodel->model);

            if (model) {
              const char *animation_name = "(brak)";
              if (omodel->animation_idx >= 0 &&
                  omodel->animation_idx < model->model.GetAnimations().size())
                animation_name =
                    model->model.GetAnimations()[omodel->animation_idx]
                        .GetName()
                        .c_str();

              ImGui::SetCursorPosX(8);
              InputAllignedText("animacja", 26);
              ImGui::SameLine();

              if (BeginCustomCombo2(&wpos, &wpos2, "animacja",
                                    ICON_FA_PERSON_WALKING, animation_name,
                                    2)) {
                int i = 0;

                if (ImGui::Selectable("(brak)", -1 == omodel->animation_idx))
                  omodel->animation_idx = -1;

                for (const auto &animation : model->model.GetAnimations()) {
                  if (ImGui::Selectable(animation.GetName().c_str(),
                                        i == omodel->animation_idx))
                    omodel->animation_idx = i;
                  i++;
                }
                EndCustomCombo(wpos, wpos2);
              }
            } else {
              ImGui::TextWrapped(
                  "animacja: (objekt nie posiada prawidłowego modelu)");
            }

            ImGui::SetCursorPosX(8);
            InputAllignedText("prędkość animacji");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 8);
            ImGui::DragFloat("##prędkość animacji", &omodel->animation_speed,
                             0.1, -FLT_MAX, FLT_MAX);

            // ManagedModel *model =
            // selected_scene->resMan.GetModel(omodel->model);
            Renderer::Animation *animation =
                model ? model->model.GetCurrentAnimation() : nullptr;

            ImGui::SetCursorPosX(8);
            InputAllignedText("czas animacji");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 8);
            ImGui::SliderFloat("##czas animacji", &omodel->animation_time, 0,
                               animation ? animation->GetDuration() : 100);

            ImGui::Dummy(ImVec2(2, 2));
          }
          if (!hidemodelprop)
            ImGui::EndChild();
          ImGui::PopStyleColor(1);
        }
      }
    }
    // }
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
          auto e1 = selected_scene->resMan.ImportModel(
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
          if (ImGui::Selectable(name.c_str(), false)) {
            selected_scene->objPool.add_model_node(
                selected_scene->resMan, selected_scene->selected_object, name);
          }
      }
      ImGui::EndChild();
    }
    ImGui::End();

    if (ImGui::Begin(ICON_FA_IMAGES " Tekstury") && selected_scene) {

      ImGui::Text(ICON_FA_IMAGE " Tekstury (%d)",
                  selected_scene->resMan.GetTexturesMap().size());
      ImGui::SameLine();

      if (ImGui::Button(" + ")) {
        std::vector<std::string> textures_to_load = ipfd::open_file(
            "Wybór obrazka", "",
            {"Plik obrazka (png/jp(e)g/gif)", "*.png;*.jpg;*.jpeg;*.gif",
             "Wszystkie pliki", "*"},
            ipfd::opt::multiselect);

        for (const auto &texture : textures_to_load) {
          auto e1 = selected_scene->resMan.ImportTexture(
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
