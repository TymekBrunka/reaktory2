#pragma once
#include <cstdint>
#include <functional>
#include <imgui.h>
#include <memory>
#include <typeinfo>
#include <vector>

class ImNodeEditor;

struct Template {
  // ImU32 headerColor;
  // ImU32 backgroundColor;
  // ImU32 backgroundColorOver;
  uint8_t inputCount = 0;
  uint8_t outputCount = 0;
  const char **inputNames = nullptr; // can be nullptr. No text displayed.
  ImU32 *inputColors =
      nullptr; // can be nullptr, default slot color will be used.
  const char **outputNames = nullptr; // can be nullptr. No text displayed.
  ImU32 *outputColors =
      nullptr; // can be nullptr, default slot color will be used.
};

struct Link {
  int8_t inputidx = -1;
  int8_t outputidx = -1;
  int32_t inputNodeidx = -1;
  int32_t outputNodeidx = -1;
};

class Node {
  bool selected = false;
  int32_t template_idx = -1;
  ImVec2 position = ImVec2(0, 0);
  ImVec2 size = ImVec2(100, 20);
};

class ImNodeEditor {};
