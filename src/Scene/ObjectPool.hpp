#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct objH {
  uint8_t gen;
  uint16_t idx;
};

struct Object {

};

class ObjectPool {
  std::vector<Object> objects;
};
