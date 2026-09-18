#include "DataBinding.hpp"
#include "Eval.hpp"
#include "Model.hpp"
#include "ResourceManager.hpp"
#include <ObjectPool.hpp>
#include <cstdint>

Object *ObjectPool::get(objH obj) {
  if (obj.idx < 0 || obj.idx >= objects.size()) {
    // Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
    //          TL(MSG_OBJECT_POOL_GET_ERROR),
    //          std::make_format_args(obj.idx, obj.gen));
    return nullptr;
  }

  uint8_t gen = objects[obj.idx].gen;
  if (gen != obj.gen) {
    // Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
    //          TL(MSG_OBJECT_POOL_GET_ERROR),
    //          std::make_format_args(obj.idx, obj.gen));
    return nullptr;
  }

  return &objects[obj.idx];
}

objH ObjectPool::add(const Object &obj) {
  int16_t idx = 0;
  if (free_indices.size() != 0) {
    idx = free_indices[free_indices.size() - 1];
    free_indices.pop_back();
  } else {
    idx = objects.size();
    objects.push_back({});
  }

  uint8_t gen = objects[idx].gen;
  objects[idx] = obj;
  objects[idx].gen = ++gen;
  return objH{.gen = gen, .idx = idx};
}

void ObjectPool::remove(objH obj) {
  if (obj.idx <= 0 || obj.idx >= objects.size()) {
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
             TL(MSG_OBJECT_POOL_REMOVE_ERROR),
             std::make_format_args(obj.idx, obj.gen));
    return;
  }

  uint8_t gen = objects[obj.idx].gen;
  if (gen != obj.gen) {
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
             TL(MSG_OBJECT_POOL_REMOVE_ERROR),
             std::make_format_args(obj.idx, obj.gen));
    return;
  }

  if (std::get_if<oEmptySlot>(&objects[obj.idx].variant)) {
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
             TL(MSG_OBJECT_POOL_REMOVE_EMPTY_SLOT_ERROR),
             std::make_format_args(obj.idx));
    return;
  }

  remove_impl(obj.idx);
}

void ObjectPool::remove_impl(int16_t idx) {
  for (int16_t child : objects[idx].children) {
    remove_impl(child);
  }
  objects[idx].variant.emplace<oEmptySlot>(oEmptySlot{});
  free_indices.push_back(idx);
}

objH ObjectPool::add_model_node(ResourceManager &resMan, objH obj,
                                const std::string_view &model_name) {

  const char *name = model_name.empty()
                         ? (*resMan.GetModelsMap().begin()).first.c_str()
                         : model_name.data();

  ManagedModel *model = resMan.GetModel(name);
  if (model == nullptr)
    return objH{.gen = 0, .idx = -1};

  auto model_name_ = std::make_shared<Eval::Model>();
  model_name_.get()->name = name;

  auto materials = std::make_shared<std::vector<Eval::Value>>();
  for (const auto &material : model->materials) {
    auto mat = std::make_shared<Eval::Material>();
    *mat.get() = Eval::Material{.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                .diffuse1 = material.diffuse1};

    materials.get()->push_back(Eval::Value{.data = mat});
  }

  oModel omodel{};
  omodel.model = model_name_;
  omodel.materials = materials;
  omodel.animation_idx = -1;
  omodel.animation_time = 0;
  omodel.animation_speed = 1.0f;

  objH child = add(Object{.name = "Model", .variant = std::move(omodel)});
  get(obj)->children.push_back(child.idx);
  return child;
}
