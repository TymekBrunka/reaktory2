#pragma once
#include <Renderer.hpp>
#include <filesystem>
#include <stdatomic.h>
#include <string>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

class Scene {
  static Renderer::rProgram skybox_program;
  static Renderer::rFBO skybox_fbo;
  static Renderer::rEBO skybox_ebo;
  static Renderer::rVAO skybox_vao;
  static Renderer::rProgram tri_program;
  static Renderer::rFBO tri_fbo;
  static Renderer::rVAO tri_vao;

  bool initialised = false;
  atomic_bool can_rename = true;
  Renderer::rect_size size{640, 480};
  Renderer::rect_size mpos{0, 0};
  // std::string current_folder;
  Renderer::rLocation skybox_loc;
  Renderer::rTexture2D skybox_texture;
  Renderer::rFBO framebuffer;
  Renderer::rRBO renderbuffer;

public:
  Renderer::rTexture2D screen_canvas;
  Renderer::rTexture2D color_canvas;

private:
  glm::vec2 input{};
  glm::vec2 orientation{};
  glm::mat4x4 camera{};
  struct {
    Renderer::rUnifomIdx camera_idx;
    Renderer::rUnifomIdx mpos_idx;
  } ubo_locs;
  std::string name;
  std::filesystem::path current_folder;

  bool create_framebuffer();
  void resize_framebuffer(Renderer::rect_size size);

  void updateCamera();

  static bool create_folder_structure(const std::string &name,
                                      const std::filesystem::path &path);

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


  inline const std::string &get_name() const { return name; }

  static bool Init(Renderer::Render &render);
  bool init(Renderer::Render &render);
  void cleanup();

  void render();

  // retuns whether it needs redraw
  bool resize(Renderer::rect_size size);
  void updateMousePos(const Renderer::rect_size &pos);
  void updateMouseButtonState(bool left, bool right);

  void rename(const std::string &name);
  // void updateSkybox(Renderer::rCubeMap skybox);
};
