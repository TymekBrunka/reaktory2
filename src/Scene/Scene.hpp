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

  bool initialised = false;
  atomic_bool can_rename = true;
  Renderer::rect_size size{640, 480};
  Renderer::rect_size mpos{0, 0};
  // std::string current_folder;
  Renderer::rCubeMap skybox_texture;
  Renderer::rFBO framebuffer;
  Renderer::rRBO renderbuffer;

public:
  Renderer::rTexture2D screen_canvas;
  Renderer::rTexture2D color_canvas;

private:
  std::string name;
  std::filesystem::path current_folder;

  bool create_framebuffer();
  void resize_framebuffer(Renderer::rect_size size);

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

  inline const std::string &get_name() const { return name; }

  static bool Init(Renderer::Render &render);
  bool init(Renderer::Render &render);
  bool cleanup();

  void render();

  // retuns whether it needs redraw
  bool resize(Renderer::rect_size size);
  void updateMousePos(const Renderer::rect_size &pos);
  void updateMouseButtonState(bool left, bool right);

  void rename(const std::string &name);
  void updateSkybox(Renderer::rCubeMap skybox);
};
