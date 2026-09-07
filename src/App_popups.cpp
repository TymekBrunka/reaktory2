#include <App.hpp>
#include <FileUtils.hpp>

void App::draw_popup() {
  static char name_buffer[200] = {0};
  static std::string owned_string;

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
}
