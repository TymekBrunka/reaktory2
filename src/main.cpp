// #include <Logging/Logging.hpp>
#include <Renderer/Renderer.hpp>
#include <icon_img.h>
#include <imgui.h>

#include <FontsAwesome/IconsFontAwesome6.h>
#include <FontsAwesome/RobotoRegular.h>
using namespace Renderer;

static ImFont *font1;

int main() {
  Render::Init();
  Render::SetTargetFPS(30);
  {
    Render r{"Reaktory", {640, 480}};

    ImFontConfig fontcfg;
    // fontcfg.PixelSnapH = true;
    // fontcfg.FontDataOwnedByAtlas = false;
    fontcfg.MergeMode = true;

    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImGuiIO &io = ImGui::GetIO();
    font1 = io.Fonts->AddFontFromMemoryCompressedTTF(
        RobotoRegular_compressed_data, RobotoRegular_compressed_size, 16.0f);
    io.Fonts->AddFontFromMemoryCompressedTTF(
        FA_compressed_data, FA_compressed_size, 16.0f, &fontcfg, icons_ranges);

    Image icon{.width = icon_png_width,
               .height = icon_png_height,
               .channels = icon_png_channels,
               .mipmap_levels = 2,
               .pixels = (unsigned char *)icon_png_pixels};

    rTexture2D icon_tex = r.LoadTexture(icon).ok_unchecked();

    r.SetWindowIcon(icon);
    while (!r.WindowShouldClose()) {
      Render::PullEvents();
      r.makeContextCurrent();
      if (!r.IsMinimised()) {
        r.BeginFrame();
        ImGui::PushFont(font1);

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::ShowDemoWindow();
        ImGui::ShowStyleEditor();
        ImGui::ShowStyleSelector("style?");

        if (ImGui::Begin("??")) {
          ImGui::Text("frame time: %.2f", r.GetDelta());
          ImGui::Image((ImTextureRef)icon_tex, ImVec2(128, 128));
        }
        ImGui::End();

        ImGui::PopFont();
        r.EndFrame();
      }
    }
  }
  Render::Cleanup();
}
