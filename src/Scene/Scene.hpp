#pragma once
#include <string>
#include <filesystem>
#include <Renderer.hpp>

class Scene {
  static Renderer::rProgram skybox_program;

  bool can_rename;
  Renderer::rect_size size;
  std::string name;
  std::filesystem::path current_folder;
  // std::string current_folder;
  Renderer::rCubeMap skybox_texture;
  Renderer::rTexture2D screen_canvas;
  Renderer::rTexture2D color_canvas;

public:
  Scene() = default;
  // ~Scene();

  void resize(Renderer::rect_size size);
  void updateMousePos(double xoffset, double yoffset);
  void updateMouseButtonState(bool left, bool right);

  void rename(const std::string &name);
  void updateSkybox(Renderer::rCubeMap skybox);
};
