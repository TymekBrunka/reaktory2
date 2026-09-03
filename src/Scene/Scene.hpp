#pragma once
#include <Model.hpp>
#include <Renderer.hpp>
#include <filesystem>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <stdatomic.h>
#include <string>

class Scene {
  static Renderer::rProgram skybox_program;
  static Renderer::rFBO skybox_fbo;
  static Renderer::rEBO skybox_ebo;
  static Renderer::rVAO skybox_vao;
  static Renderer::rProgram tri_program;
  static Renderer::rFBO tri_fbo;
  static Renderer::rVAO tri_vao;

  static Renderer::rProgram skinning_program;

  static Renderer::rLocation skybox_loc;
  static Renderer::rLocation skybox_view_loc;
  static Renderer::rLocation skybox_projection_loc;

  static Renderer::Model *preview_model;
  static Renderer::rLocation model_view_loc;
  static Renderer::rLocation model_projection_loc;

  bool initialised = false;
  bool mousebuttonL = false;
  bool mousebuttonR = false;
  atomic_bool can_rename = true;
  Renderer::rect_size size{640, 480};
  glm::vec2 last_mpos{0, 0};
  Renderer::rect_size mpos{0, 0};
  // std::string current_folder;
  Renderer::rTexture2D skybox_texture;
  Renderer::rFBO framebuffer;
  Renderer::rRBO renderbuffer;

public:
  Renderer::rTexture2D screen_canvas;
  Renderer::rTexture2D color_canvas;

private:
  glm::vec3 input{};
  // struct {
  //   Renderer::rUnifomIdx camera_idx;
  //   Renderer::rUnifomIdx mpos_idx;
  // } ubo_locs;
  struct {
    glm::vec2 orientation{};
    glm::vec3 position{};
    glm::vec3 velocity{};
    glm::vec3 acceleration{};
  } body;
  glm::mat4x4 projection{};
  glm::mat4x4 view{};
  std::string name;
  std::filesystem::path current_folder;

  bool create_framebuffer();
  void resize_framebuffer(Renderer::rect_size size);

  void updateCamera();
  void updateCameraAndBody(float delta);

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

  void render(Renderer::Render &render);

  // retuns whether it needs redraw
  bool resize(Renderer::rect_size size);

  inline void updateBodyMovement(const glm::vec3 &input_) {
    input = input_;
  }

  inline void updateMousePos(const Renderer::rect_size &pos) { mpos = pos; }

  inline void updateMouseButtonState(bool left, bool right) {
    mousebuttonL = left;
    mousebuttonR = right;
  }

  void rename(const std::string &name);
  // void updateSkybox(Renderer::rCubeMap skybox);
};
