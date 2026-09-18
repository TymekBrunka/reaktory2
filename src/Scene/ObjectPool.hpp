#pragma once
#include "Logging.hpp"
#include <DataBinding.hpp>
#include <Errors/Errors.hpp>
#include <Eval.hpp>
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

struct oModel {
  DataBinding<std::shared_ptr<Eval::Model>> model;
  DataBinding<std::shared_ptr<std::vector<Eval::Value>>> materials;
  DataBinding<int> animation_idx;
  float animation_time;
  DataBinding<float> animation_speed;
};

typedef DataBinding<Eval::objHFormula> oFormula;

struct oEmptySlot {
  bool dummy = false;
};

struct oNode {
  bool dummy = false;
};

struct Object {
  uint8_t gen = 1;
  bool collapsed = false;
  glm::mat4 transform = glm::mat4(1.0f);
  std::string name;
  std::vector<int16_t> children;
  std::variant<oEmptySlot, oNode, oFormula, oModel> variant;

  objH toHandle(int16_t idx) const { return objH{.gen = gen, .idx = idx}; }
};

class ObjectPool {
private:
  std::vector<Object> objects;
  std::vector<uint16_t> free_indices;
  void remove_impl(int16_t idx);

public:
  ObjectPool()
      : objects({Object{.gen = 1,
                        .transform = glm::mat4(1.0f),
                        .name = "Scena",
                        .children = {},
                        .variant = oNode{}}}) {};

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

  Object *get(objH obj);
  objH add(const Object &obj);
  void remove(objH obj);

  objH add_model_node(ResourceManager &resMan, objH obj, const std::string_view &model_name);
};
