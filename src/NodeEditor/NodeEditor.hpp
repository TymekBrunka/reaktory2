#pragma once
#include <Formulator.hpp>
#include <ImNodedit.hpp>

enum NodeGraph_NodeType {
  N_FORWARDER = 0,
  N_COUNT,
};

extern const ImNodedit::Template templates[N_COUNT];

// class NodeEditor : ImNodedit::ImNodeEditor {
// public:
//   NodeGraph *graph;
//   ~NodeEditor() override = default;
//
//   void selectNode(int32_t nodeIdx, bool selected) override;
//   void moveSelectedNodes(const ImVec2 delta) override;
//
//   int32_t getTemplateCount() override;
//   const ImNodedit::Template getTemplate(int32_t templateIdx) override;
//
//   int32_t getNodeCount() override;
//   ImNodedit::Node getNode(int32_t nodeIdx) override;
//
//   void drawNodeWidgets(int32_t nodeIdx, Node *node,
//                        ImNodedit::ImNodeEditor *nodedit) override;
//
//   Link getNodeInputLink(int32_t nodeIdx, int8_t pinIdx) override;
//
//   bool allowLink(int32_t inputNodeIdx, int8_t inputPinIdx,
//                  int32_t outputNodeIdx, int8_t outputPinIdx) override;
//   void addLink(int32_t inputNodeIdx, int8_t inputPinIdx, int32_t outputNodeIdx,
//                int8_t outputPinIdx) override;
//   void delLink(int32_t inputNodeIdx, int8_t inputPinIdx, int32_t outputNodeIdx,
//                int8_t outputPinIdx) override;
// };
