#include "Components/Logging/Logging.hpp"
#include "Renderer/Renderer.hpp"
#include <App.hpp>

#include <FontsAwesome/IconsFontAwesome6.h>
#include <FontsAwesome/RobotoRegular.h>
#include <icon_img.h>
#include <imgui.h>

bool App::init() {
  logger = Log::Logger{Log::LANG_PL, {Log::ConsoleLog_Callback}};
  Log::Logger::Global = &logger;

  if (!Renderer::Render::Init())
    return false;

  if (!render.init("Reaktory", {640, 480}))
    return false;

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

  icon_tex = render.LoadTexture(icon).ok_unchecked();

  render.SetWindowIcon(icon);

  return true;
}

void App::shutdown() {
  render.cleanup();
  Renderer::Render::Cleanup();
}

void App::run() {
  render.makeContextCurrent();
  while (!render.WindowShouldClose()) {
    Renderer::Render::PullEvents();
    if (!render.IsMinimised()) {
      render.BeginFrame();
      ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                   ImGuiDockNodeFlags_PassthruCentralNode);
      ImGui::ShowDemoWindow();

      if (ImGui::Begin("??" ICON_FA_BOOK)) {
        ImGui::Text("frame time: %.2f", render.GetDelta());
        ImGui::Image((ImTextureRef)icon_tex, ImVec2(128, 128));
      }
      ImGui::End();

      render.EndFrame();
    }
  }
  render.cleanup();
}
