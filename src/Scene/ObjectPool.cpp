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
