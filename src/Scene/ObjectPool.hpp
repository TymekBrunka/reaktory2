#pragma once
#include <LuaBindings.hpp>
#include <ResourceManager.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Text3D {
  struct Char {
    int charIdx;
    int colorIdx;
    int lineNr;
    int columnNr;
  };

private:
  Renderer::rVBO vbo = 0;
  Renderer::rVAO vao = 0;
  size_t v_capacity = 0;
  std::vector<Char> buf;
  int txt_len = 0;
  int length = 0;
  int height = 0;
  std::string text =
      "\\7Test :) "
      "\\1\\\\\\7n\ncz\\5es\\7c\nhej\n\\00\\11\\22\\33\\44\\55\\66\\77";

public:
  Text3D() = default;
  ~Text3D();

  Text3D(const Text3D &other);
  Text3D &operator=(const Text3D &other);
  Text3D(Text3D &&other) noexcept;
  Text3D &operator=(Text3D &&other) noexcept;

  inline const std::string &string() const { return text; }
  void setText(const char *text_);
  void draw(const glm::mat4 &pos);
};

struct objH {
  uint8_t gen = 0;
  // uint16_t idx;
  int16_t idx = -1; // signed integer so -1 can be returned

  inline bool operator==(objH other) {
    return other.gen == gen && other.idx == idx;
  }
};

struct Material {
  glm::vec4 color_diffuse{};
  std::string diffuse1;
};

struct _ModelName {
  std::string model;
};

struct _HeaderName {
  std::string header;
};

struct oModel {
  int animation_idx;
  float animation_time;
  float animation_speed;
  std::string model;
  std::vector<Material> materials;
};

struct oEmptySlot {
  bool dummy = false;
};

struct oNode {
  bool dummy = false;
};

class Sampler;
// struct oDataPoint {
//   int sheet_idx =
//       -1; // sheet index is used for determining whether to update header
//       index
//   int header_idx = -1;
//   std::string header; // key
//   Text3D display_text;
//
//   void update_text(Sampler *displayed_row);
// };

struct DPHeader {
  int header_idx = -1;
  double min = 0;
  double max = 0.5;
  std::string header; // key
};

struct oDataPoint2 {
  int sheet_idx =
      -1; // sheet index is used for determining whether to update header index
  std::vector<DPHeader> headers;
  Text3D display_text;

  void update_text(Sampler *displayed_row);
};

struct oPrefab {
  uint32_t gen = 0;
  int idx = 0;
  objH root;
};

class Scene;
class ObjectPool;

static constexpr int SCENE_SCRIPT_CLASSES_IDX = 5;
static constexpr int SCENE_DATA_IDX = 6;
static constexpr int SCENE_IN_REG_IDX = 7;

class ScriptClass {
public:
  int tableidx;
  lua_State *L;
  Scene *scene;
  std::string name;

  struct Param {
    int types = 0;
    std::string name;
  };

  std::vector<Param> params;

  ~ScriptClass() = default;
  ScriptClass(Scene *scene_, lua_State *L_, int tableidx_, const char *name_)
      : L(L_), tableidx(tableidx_), name(name_), scene(scene_) {}
  ScriptClass(const ScriptClass &other);
  ScriptClass &operator=(const ScriptClass &other);
  ScriptClass(ScriptClass &&other);
  ScriptClass &operator=(ScriptClass &&other);

  void destroy();
};

struct Nil {
  bool dummy = false;
};

using LuaParam = std::variant<Nil, bool, int, float, std::string, Material,
                              objH, _ModelName, _HeaderName>;

struct Object {
  uint8_t gen = 1;
  bool collapsed = false;
  int script_class_idx = -1;
  int ref_to_self = -1;
  objH parent = objH{.gen = 1, .idx = 0};
  glm::mat4 transform = glm::mat4(1.0f);
  glm::mat4 current_parent_transform = glm::mat4(1.0f);
  std::string name;
  std::vector<int16_t> children;
  std::vector<LuaParam> lua_params;
  std::variant<oEmptySlot, oNode, oModel, oDataPoint2, Text3D, oPrefab> variant;

  inline objH toHandle(int16_t idx) const {
    return objH{.gen = gen, .idx = idx};
  }

  void change_script_class(ObjectPool *pool, int new_script_class_idx);
};

class ObjectPool {
  friend class SceneLuaBindings;
  friend class ScriptClass;
  friend class Object;

private:
  lua_State *L;
  std::vector<uint16_t> script_data_freelist;
  std::vector<Object> objects;
  std::vector<uint16_t> free_indices;

  std::vector<int> script_class_free_list;
  std::vector<ScriptClass> script_classes;
  objH clone_impl(int16_t idx);
  void remove_impl(int16_t idx);

public:
  ObjectPool();
  ObjectPool(lua_State *L);

  ~ObjectPool() = default;
  ObjectPool(const ObjectPool &other) = delete;
  ObjectPool &operator=(const ObjectPool &other) = delete;
  ObjectPool(ObjectPool &&other) = default;
  ObjectPool &operator=(ObjectPool &&other) = default;

  inline const std::vector<ScriptClass> &get_script_classes() const {
    return script_classes;
  }

  inline lua_State *get_lua() const { return L; }

  inline objH rootH() const { return objH{.gen = objects[0].gen, .idx = 0}; }

  inline Object *getChildOf(Object *obj, int16_t idx) {
    if (obj < &objects[0] || obj > &objects[objects.size() - 1])
      return nullptr;

    if (obj->children[idx] <= 0 || obj->children[idx] >= objects.size())
      return nullptr;

    return &objects[obj->children[idx]];
  }

  inline int16_t size() const { return objects.size() - free_indices.size(); };
  inline int16_t real_size() const { return objects.size(); };

  Object *get(objH obj);
  objH add(Object &&obj);
  objH clone(objH obj);
  void remove(objH obj);

  objH add_model_node(ResourceManager &resMan, objH obj,
                      const std::string_view &model_name);

  objH add_node(objH obj);

  objH add_node(objH obj, decltype(Object::variant) &&variant);
};
