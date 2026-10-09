#include "Any_and_Pool.hpp"
#include "Formulator.hpp"
#include "ImNodedit.hpp"
#include "imgui.h"
#include <NodeEditor.hpp>
#include <imgui_stdlib.h>
#include <iostream>

void NodeEditor::selectNode(int32_t nodeIdx, bool selected) {
  // auto &idx = graph->nodes.get_indices()[nodeIdx];
  // graph->nodes.expose()[idx.idx].selected = selected;
  graph->nodes.get(tHandle{.gen = 0, .idx = nodeIdx}, true)->selected =
      selected;
}

void NodeEditor::moveSelectedNodes(const ImVec2 delta) {
  for (auto &node : graph->nodes) {
    if (node.selected) {
      node.position[0] += delta.x;
      node.position[1] += delta.y;
    }
  }
}

int32_t NodeEditor::getTemplateCount() { return N_COUNT; }

const ImNodedit::Template NodeEditor::getTemplate(int32_t templateIdx) {
  const Template &templ = templates[templateIdx];
  return ImNodedit::Template{
      .headerColor = templ.headerColor,
      .backgroundColor = templ.backgroundColor,
      .inputCount = templ.inputCount,
      .outputCount = templ.outputCount,
      .inputNames = templ.inputNames,
      .inputColors = templ.inputColors,
      .outputNames = templ.outputNames,
      .outputColors = templ.outputColors,
      .name = templ.name,
  };
}

int32_t NodeEditor::getNodeCount() { return graph->nodes.idx_size(); };

bool NodeEditor::canGetNode(int32_t nodeIdx) {
  return graph->nodes.get(tHandle{.gen = 0, .idx = nodeIdx}, true);
}

ImNodedit::Node NodeEditor::getNode(int32_t nodeIdx) {
  // auto &idx = graph->nodes.get_indices()[nodeIdx];
  // Node &node = graph->nodes.expose()[idx.idx];
  Node *node = graph->nodes.get(tHandle{.gen = 0, .idx = nodeIdx}, true);

  return ImNodedit::Node{
      .selected = node->selected,
      .templateIdx = node->templateIdx,
      .position = ImVec2((int)node->position[0] / 10 * 10,
                         (int)node->position[1] / 10 * 10),
      .size = templates[node->templateIdx].size,
      .userData =
          graph->nodeData[node->templateIdx].get()->get(node->node_data),
  };
}

void NodeEditor::drawNodeWidgets(int32_t nodeIdx, ImNodedit::Node *node,
                                 ImNodedit::ImNodeEditor *nodedit,
                                 ImDrawList *drawlist) {

  Node *node_ = graph->nodes.get(tHandle{.gen = 0, .idx = nodeIdx}, true);
  templates[node_->templateIdx].draw_function(nodeIdx, node, nodedit, drawlist);
}

ImNodedit::Link NodeEditor::getNodeInputLink(int32_t nodeIdx, int8_t pinIdx) {
  // auto &idx = graph->nodes.get_indices()[nodeIdx];
  // Node &node_ = graph->nodes.expose()[idx.idx];
  Node *node_ = graph->nodes.get(tHandle{.gen = 0, .idx = nodeIdx}, true);

  NodePoolBase *nodeData = graph->nodeData[node_->templateIdx].get();
  Link *inputs = nodeData->inputsOf(nodeData->get(node_->node_data));
  return ImNodedit::Link{
      inputs[pinIdx].pinIdx,
      inputs[pinIdx].nodeIdx,
  };
}

bool NodeEditor::allowLink(int32_t inputNodeIdx, int8_t inputPinIdx,
                           int32_t outputNodeIdx, int8_t outputPinIdx) {
  return true;
}

void NodeEditor::addLink(int32_t inputNodeIdx, int8_t inputPinIdx,
                         int32_t outputNodeIdx, int8_t outputPinIdx) {
  // auto &idx = graph->nodes.get_indices()[inputNodeIdx];
  // Node &node_ = graph->nodes.expose()[idx.idx];
  Node *node_ = graph->nodes.get(tHandle{.gen = 0, .idx = inputNodeIdx}, true);

  NodePoolBase *nodeData = graph->nodeData[node_->templateIdx].get();
  Link *inputs = nodeData->inputsOf(nodeData->get(node_->node_data));

  inputs[inputPinIdx] = Link{.pinIdx = outputPinIdx, .nodeIdx = outputNodeIdx};
}

void NodeEditor::delLink(int32_t inputNodeIdx, int8_t inputPinIdx,
                         int32_t outputNodeIdx, int8_t outputPinIdx) {
  // auto &idx = graph->nodes.get_indices()[inputNodeIdx];
  // Node &node_ = graph->nodes.expose()[idx.idx];
  Node *node_ = graph->nodes.get(tHandle{.gen = 0, .idx = inputNodeIdx}, true);

  NodePoolBase *nodeData = graph->nodeData[node_->templateIdx].get();
  Link *inputs = nodeData->inputsOf(nodeData->get(node_->node_data));

  inputs[inputPinIdx] = Link{-1, -1};
}

// <- names

static const char *N_FORWARDER__inNames[] = {"z  "};
static const char *N_FORWARDER__outNames[] = {"do  "};

// static const char *N_NUMBER__outNames[] = {"liczba  "};
// static const char *N_STRING__outNames[] = {"ciąg znaków  "};

// static const char *N_aritm_op__inNames[] = {"a  ", "b  "};
// static const char *N_aritm_op__outNames[] = {"wynik  "};

static const char *N_SUB__inNames[] = {"odjemna", "odjemnik"};

static const char *N_DIV__inNames[] = {"dzielna", "dzielnik"};

static const char *N_IF__inNames[] = {"gdy prawda", "warunek", "gdy fałsz"};

static const char *N_LOOP__inNames[] = {"war. pocz.", "gdy prawda", "warunek"};
static const char *N_LOOP__outNames[] = {"x  ", "wynik "};

static const char *N_RETURN__inNames[] = {"wynik  "};

// <- colors
static const ImU32 COLOR_NUMBER = ImNodedit::imColor(0x7BBC2BFF);
static const ImU32 COLOR_HDR_ARITM_OP = ImNodedit::imColor(0x51A019FF);
static const ImU32 COLOR_BG_ARITM_OP = ImNodedit::imColor(0x293323FF);
static const ImU32 N_NUMBER__outColors[] = {COLOR_NUMBER};
static const ImU32 N_aritm_op__inColors[] = {COLOR_NUMBER, COLOR_NUMBER};
static const ImU32 N_aritm_op__outColors[] = {COLOR_NUMBER};

static const ImU32 COLOR_STRING = ImNodedit::imColor(0xE2461BFF);
static const ImU32 N_STRING__outColors[] = {COLOR_STRING};

static const ImU32 COLOR_BOOL = ImNodedit::imColor(0x377899FF);
static const ImU32 COLOR_HDR_BOOL = ImNodedit::imColor(0x377899FF);
static const ImU32 COLOR_BG_BOOL = ImNodedit::imColor(0x2B3F49FF);
static const ImU32 N_BOOL__outColors[] = {COLOR_BOOL};

static const ImU32 N_RETURN__inColors[] = {ImNodedit::imColor(0x68002FFF)};

// <- templates

void TextSized(ImDrawList *drawlist, float zoom, ImVec2 offset, ImU32 color,
               const char *text) {
  ImVec2 cpos = ImGui::GetCursorScreenPos();
  float fontscale = ImGui::GetIO().FontGlobalScale;
  ImGui::GetIO().FontGlobalScale = fontscale * 3;
  ImGui::BeginChild(1, ImVec2(10 * zoom, 0));
  drawlist->AddText(ImVec2(cpos.x + (offset.x * zoom),
                           cpos.y + (offset.y * zoom) - (14 * zoom)),
                    color, text);
  ImGui::EndChild();
  ImGui::GetIO().FontGlobalScale = fontscale;
  ImGui::SameLine();
  ImGui::BeginChild(2, ImVec2(1, 1));
  ImGui::EndChild();
}

template <typename T>
void IconPlusValueDisabledOnLink(ImDrawList *drawlist, ImNodedit::Node *node,
                                 ImNodedit::ImNodeEditor *nodedit,
                                 ImVec2 offset, const char *text) {
  float zoom = nodedit->getZoomFactor();
  TextSized(drawlist, zoom, offset, 0xffffffff, text);
  ImGui::SameLine();

  auto *data = (T *)node->userData;
  if (data->inputs[1] != Link{-1, -1})
    ImGui::BeginDisabled();

  ImGui::SetNextItemWidth(ImGui::GetWindowSize().x -
                          (10 * nodedit->getZoomFactor()));
  ImGui::DragFloat("##number", &data->value, 0.1);

  if (data->inputs[1] != Link{-1, -1})
    ImGui::EndDisabled();
}

extern const Template templates[N_COUNT]{
    Template{.headerColor = ImNodedit::imColor(0x333A3DFF),
             .size = ImVec2(80, 50),
             .inputCount = 1,
             .outputCount = 1,
             .inputNames = N_FORWARDER__inNames,
             .outputNames = N_FORWARDER__outNames,
             .name = "przekaźnik",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   // ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), "%.0f ,
                   // %.0f",
                   //                    node->position.x / 10,
                   //                    node->position.y / 10);
                 }},

    Template{.headerColor = ImNodedit::imColor(0x377899FF),
             .backgroundColor = ImNodedit::imColor(0x70C3FFFF),
             .size = ImVec2(70, 60),
             .inputCount = 0,
             .outputCount = 1,
             .outputColors = N_BOOL__outColors,
             .name = "logiczna",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   ImGui::Checkbox("##bool", &((nBool *)node->userData)->value);
                 }},

    Template{.headerColor = ImNodedit::imColor(0x416B18FF),
             .backgroundColor = ImNodedit::imColor(0x51A019FF),
             .size = ImVec2(120, 60),
             .inputCount = 0,
             .outputCount = 1,
             // .inputNames = N_FORWARDER__inNames,
             // .outputNames = N_NUMBER__outNames,
             .outputColors = N_NUMBER__outColors,
             .name = "liczba",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   ImVec2 img_size = ImVec2(20 * nodedit->getZoomFactor(),
                                            20 * nodedit->getZoomFactor());

                   TextSized(drawlist, nodedit->getZoomFactor(), ImVec2(0, 0),
                             ImNodedit::imColor(0x0055AAFF), "#");
                   ImGui::SameLine();

                   ImGui::SetNextItemWidth(ImGui::GetWindowSize().x -
                                           img_size.x);
                   ImGui::DragFloat("##number",
                                    &((nNumber *)node->userData)->value, 0.1);
                 }},

    Template{.headerColor = ImNodedit::imColor(0xCC4A00FF),
             .backgroundColor = ImNodedit::imColor(0xFF7F00FF),
             .size = ImVec2(120, 60),
             .inputCount = 0,
             .outputCount = 1,
             .outputColors = N_STRING__outColors,
             .name = "ciąg znaków",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   ImGui::SetNextItemWidth(ImGui::GetWindowSize().x);
                   ImGui::InputText("##string",
                                    &((nString *)node->userData)->value);
                 }},

    Template{.headerColor = COLOR_HDR_ARITM_OP,
             .backgroundColor = COLOR_BG_ARITM_OP,
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             // .inputNames = N_FORWARDER__inNames,
             .inputColors = N_aritm_op__inColors,
             // .outputNames = N_STRING__outNames,
             .outputColors = N_aritm_op__outColors,
             .name = "dodaj (+)",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   IconPlusValueDisabledOnLink<nAdd>(drawlist, node, nodedit,
                                                     ImVec2(0, 0), "+");
                 }},

    Template{.headerColor = COLOR_HDR_ARITM_OP,
             .backgroundColor = COLOR_BG_ARITM_OP,
             .size = ImVec2(170, 60),
             .inputCount = 2,
             .outputCount = 1,
             .inputNames = N_SUB__inNames,
             .inputColors = N_aritm_op__inColors,
             // .outputNames = N_STRING__outNames,
             .outputColors = N_aritm_op__outColors,
             .name = "odejmij (-)",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   IconPlusValueDisabledOnLink<nSub>(drawlist, node, nodedit,
                                                     ImVec2(0, 0), "-");
                 }},

    Template{.headerColor = COLOR_HDR_ARITM_OP,
             .backgroundColor = COLOR_BG_ARITM_OP,
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             // .inputNames = N_FORWARDER__inNames,
             .inputColors = N_aritm_op__inColors,
             // .outputNames = N_STRING__outNames,
             .outputColors = N_aritm_op__outColors,
             .name = "pomnóż (*)",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   IconPlusValueDisabledOnLink<nMul>(drawlist, node, nodedit,
                                                     ImVec2(0, 5), "*");
                 }},

    Template{.headerColor = COLOR_HDR_ARITM_OP,
             .backgroundColor = COLOR_BG_ARITM_OP,
             .size = ImVec2(170, 60),
             .inputCount = 2,
             .outputCount = 1,
             .inputNames = N_DIV__inNames,
             .inputColors = N_aritm_op__inColors,
             // .outputNames = N_STRING__outNames,
             .outputColors = N_aritm_op__outColors,
             .name = "podziel (/)",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   IconPlusValueDisabledOnLink<nDiv>(drawlist, node, nodedit,
                                                     ImVec2(0, 0), "/");
                 }},

    Template{.headerColor = COLOR_HDR_BOOL,
             .backgroundColor = COLOR_BG_BOOL,
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             .outputColors = N_BOOL__outColors,
             .name = "=",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   IconPlusValueDisabledOnLink<nEqual>(drawlist, node, nodedit,
                                                       ImVec2(0, 0), "=");
                 }},

    Template{.headerColor = COLOR_HDR_BOOL,
             .backgroundColor = COLOR_BG_BOOL,
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             .outputColors = N_BOOL__outColors,
             .name = "<",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   IconPlusValueDisabledOnLink<nLess>(drawlist, node, nodedit,
                                                      ImVec2(0, 0), "<");
                 }},

    Template{.headerColor = COLOR_HDR_BOOL,
             .backgroundColor = COLOR_BG_BOOL,
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             .outputColors = N_BOOL__outColors,
             .name = ">",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   IconPlusValueDisabledOnLink<nMore>(drawlist, node, nodedit,
                                                      ImVec2(0, 0), ">");
                 }},

    Template{.headerColor = COLOR_HDR_BOOL,
             .backgroundColor = COLOR_BG_BOOL,
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             .outputColors = N_BOOL__outColors,
             .name = "≤",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   IconPlusValueDisabledOnLink<nLessOrEqual>(
                       drawlist, node, nodedit, ImVec2(0, -2), "≤");
                 }},

    Template{.headerColor = COLOR_HDR_BOOL,
             .backgroundColor = COLOR_BG_BOOL,
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             .outputColors = N_BOOL__outColors,
             .name = "≥",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   IconPlusValueDisabledOnLink<nMoreOrEqual>(
                       drawlist, node, nodedit, ImVec2(0, -2), "≥");
                 }},

    Template{.headerColor = COLOR_HDR_BOOL,
             .backgroundColor = COLOR_BG_BOOL,
             .size = ImVec2(40, 60),
             .inputCount = 1,
             .outputCount = 1,
             .outputColors = N_BOOL__outColors,
             .name = "nie",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   TextSized(drawlist, nodedit->getZoomFactor(), ImVec2(0, -5),
                             0xffffffff, "!");
                 }},

    Template{.headerColor = COLOR_HDR_BOOL,
             .backgroundColor = COLOR_BG_BOOL,
             .size = ImVec2(140, 100),
             .inputCount = 3,
             .outputCount = 1,
             .inputNames = N_IF__inNames,
             // .inputColors = N_aritm_op__inColors,
             // .outputNames = N_STRING__outNames,
             // .outputColors = N_aritm_op__outColors,
             .name = "jeżeli",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   TextSized(drawlist, nodedit->getZoomFactor(), ImVec2(5, 20),
                             0xffffffff, "?");
                 }},

    Template{.headerColor = COLOR_HDR_BOOL,
             .backgroundColor = COLOR_BG_BOOL,
             .size = ImVec2(180, 100),
             .inputCount = 3,
             .outputCount = 2,
             .inputNames = N_LOOP__inNames,
             .outputNames = N_LOOP__outNames,
             .name = "pętla",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   TextSized(drawlist, nodedit->getZoomFactor(), ImVec2(0, 20),
                             0xffffffff, "→");
                 }},

    Template{.headerColor = ImNodedit::imColor(0xFF0043FF),
             .backgroundColor = ImNodedit::imColor(0xAD002EFF),
             .size = ImVec2(120, 60),
             .inputCount = 1,
             .outputCount = 0,
             .inputColors = N_RETURN__inColors,
             .name = "wynik",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   ImGui::TextColored(ImVec4(1, 0.625, 0.703125, 1),
                                      "wartość zwracana");
                 }},
};
