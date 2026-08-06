#pragma once
#include <string>
#include <Renderer>

class Scene {
  static Renderer::rProgram skybox_program;

  Renderer::rect_size size;
  std::string name;
  Renderer::rCubeMap skybox_texture;
  Renderer::rTexture2D screen_canvas;
  Renderer::rTexture2D color_canvas;

public:
  Scene() = default;
  ~Scene();

  void resize(Renderer::rect_size size);
  void updateMousePos(double xoffset, double yoffset);
  void updateMouseButtonState(bool left, bool right);

  void rename(const std::string &name);
  void updateSkybox(Renderer::rCubeMap skybox);
};
