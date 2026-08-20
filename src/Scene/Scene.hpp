#pragma once
#include <Renderer.hpp>
#include <filesystem>
#include <stdatomic.h>
#include <string>

class Scene {
  static Renderer::rProgram skybox_program;
  static Renderer::rProgram tri_program;
  static Renderer::rFBO tri_fbo;
  static Renderer::rVAO tri_vao;

  bool initialised;
  atomic_bool can_rename = true;
  Renderer::rect_size size;
  // std::string current_folder;
  Renderer::rCubeMap skybox_texture;
  Renderer::rFBO framebuffer;
  Renderer::rRBO renderbuffer;
  Renderer::rTexture2D screen_canvas;
  Renderer::rTexture2D color_canvas;
  std::string name;
  std::filesystem::path current_folder;

  bool create_framebuffer();
  bool resize_framebuffer(Renderer::rect_size size);

public:
  Scene() = default;
  Scene(const std::string &name);
  Scene(const Scene &other) = delete;
  Scene &operator=(const Scene &other) = delete;
  Scene(Scene &&other);
  Scene &operator=(Scene &&other);
  ~Scene();

  inline bool is_initialised() const { return initialised; }
  static void Cleanup();

  static bool create_folder_structure(const std::string &name,
                                      const std::filesystem::path &path);

  inline const std::string &get_name() const {
    return name;
  }

  bool init(Renderer::Render &render);
  bool cleanup();

  void render();

  void resize(Renderer::rect_size size);
  void updateMousePos(double xoffset, double yoffset);
  void updateMouseButtonState(bool left, bool right);

  void rename(const std::string &name);
  void updateSkybox(Renderer::rCubeMap skybox);
};
