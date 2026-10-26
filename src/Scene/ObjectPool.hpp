#pragma once
#include <ResourceManager.hpp>
#include <cstdint>
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

struct Object {
  uint8_t gen = 1;
  bool collapsed = false;
  int script_class_idx = -1;
  int ref_to_self = -1;
  objH parent = objH{.gen = 1, .idx = 0};
  glm::mat4 transform = glm::mat4(1.0f);
  std::string name;
  std::vector<int16_t> children;
  std::vector<int> lua_params;
  std::variant<oEmptySlot, oNode, oModel> variant;

  inline objH toHandle(int16_t idx) const { return objH{.gen = gen, .idx = idx}; }

  void change_script_class(Scene *scene, int new_script_class_idx);
};

class ObjectPool {
private:
  std::vector<Object> objects;
  std::vector<uint16_t> free_indices;
  void remove_impl(int16_t idx);

public:
  ObjectPool();

  ~ObjectPool() = default;
  ObjectPool(const ObjectPool &other) = delete;
  ObjectPool &operator=(const ObjectPool &other) = delete;
  ObjectPool(ObjectPool &&other) = default;
  ObjectPool &operator=(ObjectPool &&other) = default;

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
