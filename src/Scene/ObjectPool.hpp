#pragma once
#include <LuaBindings.hpp>
#include <ResourceManager.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct objH {
  uint8_t gen;
  // uint16_t idx;
  int16_t idx; // signed integer so -1 can be returned

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

struct oModel {
  std::string model;
  std::vector<Material> materials;
  int animation_idx;
  float animation_time;
  float animation_speed;
};

struct oEmptySlot {
  bool dummy = false;
};

struct oNode {
  bool dummy = false;
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
  Scene **scene;
  std::string name;

  struct Param {
    int types = 0;
    std::string name;
  };

  std::vector<Param> params;

  ~ScriptClass() = default;
  ScriptClass(Scene **scene_, lua_State *L_, int tableidx_, const char *name_)
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
                              objH, _ModelName>;

struct Object {
  uint8_t gen = 1;
  bool collapsed = false;
  int script_class_idx = -1;
  int ref_to_self = -1;
  objH parent = objH{.gen = 1, .idx = 0};
  glm::mat4 transform = glm::mat4(1.0f);
  std::string name;
  std::vector<int16_t> children;
  std::vector<LuaParam> lua_params;
  std::variant<oEmptySlot, oNode, oModel> variant;

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
  objH add(const Object &obj);
  void remove(objH obj);

  objH add_model_node(ResourceManager &resMan, objH obj,
                      const std::string_view &model_name);
};
