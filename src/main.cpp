// #include <Logging/Logging.hpp>
#include <Renderer/Renderer.hpp>
#include <icon_img.h>
#include <imgui.h>
using namespace Renderer;

int main() {
  Render::Init();
  Render r{"reaktory", {640, 480}};

  Image icon = {.width = icon_png_width,
                .height = icon_png_height,
                .channels = icon_png_channels,
                .pixels = (unsigned char *)icon_png_pixels};

  r.SetWindowIcon(icon);
  r.makeContextCurrent();
  while (!r.WindowShouldClose()) {
    r.BeginFrame();

    ImGui::DockSpaceOverViewport();
    ImGui::ShowDemoWindow();

    r.EndFrame();
  }
  Render::Cleanup();
}
