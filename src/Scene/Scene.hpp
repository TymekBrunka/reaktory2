#pragma once
#include <Model.hpp>
#include <Renderer.hpp>
#include <filesystem>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <stdatomic.h>
#include <string>

#include <LuaBindings.hpp>
#include <ObjectPool.hpp>
#include <ResourceManager.hpp>
#include <Sheet.hpp>

struct DisplayRow {
  bool emit_update = false;
  int width = 0;
  std::string sheet_name;
  SheetRow row;
  const CellValue *ptr = nullptr;
};

class Scene {
  friend class SceneLuaBindings;
  friend class ScriptClass;
  static bool render_selection_bb;
  static Renderer::rProgram skybox_program;
  static Renderer::rFBO skybox_fbo;
  static Renderer::rEBO skybox_ebo;
  static Renderer::rVAO skybox_vao;
  static Renderer::rProgram selection_bb_program;
  static Renderer::rFBO selection_bb_fbo;
  static Renderer::rVAO selection_bb_vao;
  static Renderer::rProgram tri_program;
  static Renderer::rFBO tri_fbo;
  static Renderer::rVAO tri_vao;

  static Renderer::rProgram skinning_program;

  static Renderer::rLocation skybox_loc;
  static Renderer::rLocation skybox_view_loc;
  static Renderer::rLocation skybox_projection_loc;

  static Renderer::rLocation model_view_loc;
  static Renderer::rLocation model_projection_loc;
  static Renderer::rLocation model_ID_loc;
  static Renderer::rLocation model_model_loc;

  static Renderer::rLocation selection_bb_view_loc;
  static Renderer::rLocation selection_bb_projection_loc;
  static Renderer::rLocation selection_bb_model_loc;
  static Renderer::rLocation selection_bb_min_loc;
  static Renderer::rLocation selection_bb_max_loc;

  bool initialised = false;
  bool mousebuttonL = false;
  bool mousebuttonR = false;
  bool mousebuttonLclick = false;
  atomic_bool is_busy = true;
  Renderer::rect_size size{640, 480};
  glm::vec2 last_mpos{0, 0};
  Renderer::rect_size mpos{0, 0};
  // std::string current_folder;
  Renderer::rTexture2D skybox_texture = 0;
  Renderer::rFBO framebuffer = 0;
  Renderer::rRBO renderbuffer = 0;

public:
  Renderer::rTexture2D screen_canvas = 0;
  Renderer::rTexture2D color_canvas = 0;
  objH selected_object = {.gen = 1, .idx = 0}; // scene node
  std::string selected_sheet;

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
  std::filesystem::path current_folder{};

  lua_State *L;

  struct string_hash {
    using is_transparent = void;
    [[nodiscard]] inline size_t operator()(const char *txt) const {
      return std::hash<std::string_view>{}(txt);
    }
    [[nodiscard]] inline size_t operator()(std::string_view txt) const {
      return std::hash<std::string_view>{}(txt);
    }
    [[nodiscard]] inline size_t operator()(const std::string &txt) const {
      return std::hash<std::string>{}(txt);
    }
  };

  // std::unordered_map<std::string, Sheet, string_hash, std::equal_to<>>
  // sheets;
  std::unordered_map<std::string, Sheet> sheets;

  bool create_framebuffer();
  void resize_framebuffer(Renderer::rect_size size);

  void updateCamera();
  void updateCameraAndBody(float delta);

  static bool create_folder_structure(const std::string &name,
                                      const std::filesystem::path &path);

  void draw_models_recursive(int16_t idx, float delta,
                             glm::mat4 parentTransform, objH parentObj);

  void add_lua(lua_State *L);

public:
  ResourceManager resMan{};
  ObjectPool objPool{};

  DisplayRow displayed_row;

  Scene() = default;
  Scene(const std::string &name);
  Scene(const Scene &other) = delete;
  Scene &operator=(const Scene &other) = delete;
  // Scene(Scene &&other);
  // Scene &operator=(Scene &&other);
  ~Scene();

  inline bool is_initialised() const { return initialised; }
  static void Cleanup();

  inline const std::string &get_name() const { return name; }

  inline const decltype(sheets) &get_sheets() const { return sheets; }

  static bool Init(Renderer::Render &render);
  bool init(Renderer::Render &render);
  void cleanup();

  void render(Renderer::Render &render);

  // retuns whether it needs redraw
  bool resize(Renderer::rect_size size);

  inline void updateBodyMovement(const glm::vec3 &input_) { input = input_; }

  inline void updateMousePos(const Renderer::rect_size &pos) { mpos = pos; }

  inline void updateMouseButtonState(bool left, bool right) {
    mousebuttonLclick = left && !mousebuttonL;
    mousebuttonL = left;
    mousebuttonR = right;
  }

  void rename(const std::string &name);
  // void updateSkybox(Renderer::rCubeMap skybox);
};
