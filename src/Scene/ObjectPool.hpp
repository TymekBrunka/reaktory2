#pragma once
#include <ResourceManager.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <DataBinding.hpp>

struct objH {
  uint8_t gen;
  // uint16_t idx;
  int16_t idx; // signed integer so -1 can be returned
};

struct oModel {
  std::string model;
};

struct Object {
  uint8_t gen;
  glm::mat4 transform;
  std::string name;
  std::vector<objH> children;
};

class ObjectPool {
  std::vector<Object> objects;
  std::vector<uint16_t> free_indices;
};
