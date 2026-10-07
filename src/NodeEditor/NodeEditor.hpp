#pragma once
#include "Any_and_Pool.hpp"
#include "imgui.h"
#include <Formulator.hpp>
#include <ImNodedit.hpp>

struct Template {
  ImU32 headerColor = ImNodedit::imColor(0x35A544FF);
  ImU32 backgroundColor = ImNodedit::imColor(0x24292BFF);
  ImVec2 size = ImVec2(250, 100);
  // ImU32 backgroundColorOver;
  uint8_t inputCount = 0;
  uint8_t outputCount = 0;
  const char **inputNames = nullptr; // can be nullptr. No text displayed.
  const ImU32 *inputColors =
      nullptr; // can be nullptr, default slot color will be used.
  const char **outputNames = nullptr; // can be nullptr. No text displayed.
  const ImU32 *outputColors =
      nullptr; // can be nullptr, default slot color will be used.

  const char *name = "example node";
  std::function<void(int32_t, ImNodedit::Node *, ImNodedit::ImNodeEditor *,
                     ImDrawList *)>
      draw_function;
};

extern const Template templates[N_COUNT];

class NodeEditor : public ImNodedit::ImNodeEditor {
public:
  NodeGraph *graph;

  NodeEditor() = default;
  NodeEditor(NodeGraph *ng) : graph(ng) {}
  ~NodeEditor() override = default;

  void selectNode(int32_t nodeIdx, bool selected) override;
  void moveSelectedNodes(const ImVec2 delta) override;

  int32_t getTemplateCount() override;
  const ImNodedit::Template getTemplate(int32_t templateIdx) override;

  int32_t getNodeCount() override;
  ImNodedit::Node getNode(int32_t nodeIdx) override;

  void drawNodeWidgets(int32_t nodeIdx, ImNodedit::Node *node,
                       ImNodedit::ImNodeEditor *nodedit,
                       ImDrawList *drawlist) override;

  ImNodedit::Link getNodeInputLink(int32_t nodeIdx, int8_t pinIdx) override;

  bool allowLink(int32_t inputNodeIdx, int8_t inputPinIdx,
                 int32_t outputNodeIdx, int8_t outputPinIdx) override;

  void addLink(int32_t inputNodeIdx, int8_t inputPinIdx, int32_t outputNodeIdx,
               int8_t outputPinIdx) override;

  void delLink(int32_t inputNodeIdx, int8_t inputPinIdx, int32_t outputNodeIdx,
               int8_t outputPinIdx) override;
};
