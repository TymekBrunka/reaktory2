#pragma once
#include <cstdint>
#include <functional>
#include <imgui.h>
#include <memory>
#include <typeinfo>
#include <vector>
namespace ImNodedit {

inline constexpr ImU32 imColor(int32_t x) {
  uint8_t *in = (uint8_t *)&x;
  uint8_t out[4] = {in[3], in[2], in[1], in[0]};
  return *(ImU32 *)&out;
};

inline ImU32 imColorBlendRGBA(ImU32 x, ImU32 y, float factor) {
  uint8_t *a = (uint8_t *)&x;
  uint8_t *b = (uint8_t *)&y;
  uint8_t out[4] = {
      (uint8_t)(a[0] + ((float)(b[0] - a[0]) * factor)),
      (uint8_t)(a[1] + ((float)(b[1] - a[1]) * factor)),
      (uint8_t)(a[2] + ((float)(b[2] - a[2]) * factor)),
      (uint8_t)(a[3] + ((float)(b[3] - a[3]) * factor)),
  };
  return *(ImU32 *)&out;
}

class ImNodeEditor;

struct Template {
  ImU32 headerColor = imColor(0x35A544FF);
  ImU32 backgroundColor = imColor(0x24292BFF);
  // ImU32 backgroundColorOver;
  uint8_t inputCount = 0;
  uint8_t outputCount = 0;
  const char **inputNames = nullptr; // can be nullptr. No text displayed.
  ImU32 *inputColors =
      nullptr; // can be nullptr, default slot color will be used.
  const char **outputNames = nullptr; // can be nullptr. No text displayed.
  ImU32 *outputColors =
      nullptr; // can be nullptr, default slot color will be used.

  const char *name = "example node";
};

// struct Link {
//   int8_t inputPinIdx = -1;
//   int8_t outputPinIdx = -1;
//   int32_t inputNodeIdx = -1;
//   int32_t outputNodeIdx = -1;
// };

struct Link {
  int8_t pinIdx = -1;
  int32_t nodeIdx = -1;
};

struct Node {
  bool selected = false;
  bool hovered = false;
  int32_t templateIdx = -1;
  ImVec2 position = ImVec2(0, 0);
  ImVec2 size = ImVec2(100, 20);
  void *userData = nullptr;
};

class ImNodeEditor {
protected:
  float zoom = 1;
  bool is_focused = false;
  bool is_moving_with_mouse = false;
  bool is_it_output_pin = false;
  ImVec2 offset = ImVec2(0, 0);
  ImVec2 global_wsize = ImVec2(0, 0);
  ImVec2 global_wpos = ImVec2(0, 0);
  Link currently_dragged_pin = {-1, -1};
  Link currently_selected_link[2] = {{-1, -1}, {-1, -1}};

  virtual ~ImNodeEditor() = default;

  virtual void selectNode(int32_t nodeIdx, bool selected) = 0;
  virtual void moveSelectedNodes(const ImVec2 delta) = 0;

  virtual int32_t getTemplateCount() = 0;
  virtual const Template getTemplate(int32_t templateIdx) = 0;

  virtual int32_t getNodeCount() = 0;
  virtual Node getNode(int32_t nodeIdx) = 0;

  virtual void drawNodeWidgets(int32_t nodeIdx, Node *node,
                               ImNodeEditor *nodedit) = 0;

  virtual Link getNodeInputLink(int32_t nodeIdx, int8_t pinIdx) = 0;
  // virtual int32_t getNodeOutputPinLinksNum(int32_t nodeIdx, int8_t pinIdx) =
  // 0; virtual Link getNodeOutputLink(int32_t nodeIdx, int8_t pinIdx, int32_t
  // outputIdx) = 0;

  virtual bool allowLink(int32_t inputNodeIdx, int8_t inputPinIdx,
                         int32_t outputNodeIdx, int8_t outputPinIdx) = 0;
  virtual void addLink(int32_t inputNodeIdx, int8_t inputPinIdx,
                       int32_t outputNodeIdx, int8_t outputPinIdx) = 0;
  virtual void delLink(int32_t inputNodeIdx, int8_t inputPinIdx,
                       int32_t outputNodeIdx, int8_t outputPinIdx) = 0;

  ImVec2 getInputPinPos(const Node &node, const Template &templ,
                        int8_t pinIdx) const;

  ImVec2 getOutputPinPos(const Node &node, const Template &templ,
                         int8_t pinIdx) const;

public:
  ImVec2 screen2world(ImVec2 vec);
  ImVec2 world2screen(ImVec2 vec);
  inline void setSize(ImVec2 size) { global_wsize = size; }
  inline bool isFocused() const { return is_focused; }
  void update(const char *name = "ImNodedit");

  inline float getZoomFactor() const { return zoom; }
};

} // namespace ImNodedit
