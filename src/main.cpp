// #include <Logging/Logging.hpp>
#include <Renderer/Renderer.hpp>
using namespace Renderer;

int main() { 
  Render::Init();
  while (!Render::WindowShouldClose()) {
    Render::BeginFrame();

    Render::EndFrame();
  }
  Render::Cleanup();
}
