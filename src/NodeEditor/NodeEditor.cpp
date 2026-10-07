#include "Formulator.hpp"
#include "ImNodedit.hpp"
#include "imgui.h"
#include <NodeEditor.hpp>
#include <imgui_stdlib.h>
#include <iostream>

void NodeEditor::selectNode(int32_t nodeIdx, bool selected) {
  auto &idx = graph->nodes.get_indices()[nodeIdx];
  graph->nodes.expose()[idx.idx].selected = selected;
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

int32_t NodeEditor::getNodeCount() { return graph->nodes.size(); };

ImNodedit::Node NodeEditor::getNode(int32_t nodeIdx) {
  auto &idx = graph->nodes.get_indices()[nodeIdx];
  Node &node = graph->nodes.expose()[idx.idx];

  return ImNodedit::Node{
      .selected = node.selected,
      .templateIdx = node.templateIdx,
      .position = ImVec2((int)node.position[0] / 10 * 10,
                         (int)node.position[1] / 10 * 10),
      .size = templates[node.templateIdx].size,
      .userData = graph->nodeData[node.templateIdx].get()->get(node.node_data),
  };
}

void NodeEditor::drawNodeWidgets(int32_t nodeIdx, ImNodedit::Node *node,
                                 ImNodedit::ImNodeEditor *nodedit,
                                 ImDrawList *drawlist) {
  auto &idx = graph->nodes.get_indices()[nodeIdx];
  Node &node_ = graph->nodes.expose()[idx.idx];
  templates[node_.templateIdx].draw_function(nodeIdx, node, nodedit, drawlist);
}

ImNodedit::Link NodeEditor::getNodeInputLink(int32_t nodeIdx, int8_t pinIdx) {
  auto &idx = graph->nodes.get_indices()[nodeIdx];
  Node &node_ = graph->nodes.expose()[idx.idx];

  NodePoolBase *nodeData = graph->nodeData[node_.templateIdx].get();
  Link *inputs = nodeData->inputsOf(nodeData->get(node_.node_data));
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
  auto &idx = graph->nodes.get_indices()[inputNodeIdx];
  Node &node_ = graph->nodes.expose()[idx.idx];

  NodePoolBase *nodeData = graph->nodeData[node_.templateIdx].get();
  Link *inputs = nodeData->inputsOf(nodeData->get(node_.node_data));

  inputs[inputPinIdx] = Link{.pinIdx = outputPinIdx, .nodeIdx = outputNodeIdx};
}

void NodeEditor::delLink(int32_t inputNodeIdx, int8_t inputPinIdx,
                         int32_t outputNodeIdx, int8_t outputPinIdx) {
  auto &idx = graph->nodes.get_indices()[inputNodeIdx];
  Node &node_ = graph->nodes.expose()[idx.idx];

  NodePoolBase *nodeData = graph->nodeData[node_.templateIdx].get();
  Link *inputs = nodeData->inputsOf(nodeData->get(node_.node_data));

  inputs[inputPinIdx] = Link{-1, -1};
}

// <- names

static const char *N_FORWARDER__inNames[] = {"z  "};
static const char *N_FORWARDER__outNames[] = {"do  "};

// static const char *N_NUMBER__outNames[] = {"liczba  "};
// static const char *N_STRING__outNames[] = {"ciąg znaków  "};

// static const char *N_aritm_op__inNames[] = {"a  ", "b  "};
// static const char *N_aritm_op__outNames[] = {"wynik  "};

static const char *N_RETURN__inNames[] = {"wynik  "};

// <- colors

static const ImU32 COLOR_NUMBER = ImNodedit::imColor(0x7BBC2BFF);

static const ImU32 N_NUMBER__outColors[] = {COLOR_NUMBER};

static const ImU32 N_aritm_op__inColors[] = {COLOR_NUMBER, COLOR_NUMBER};
static const ImU32 N_aritm_op__outColors[] = {COLOR_NUMBER};

static const ImU32 N_RETURN__inColors[] = {ImNodedit::imColor(0x68002FFF)};

// <- templates

void TextSized(ImDrawList *drawlist, float zoom, const char *text) {
  ImVec2 cpos = ImGui::GetCursorScreenPos();
  float fontscale = ImGui::GetIO().FontGlobalScale;
  ImGui::GetIO().FontGlobalScale = fontscale * 3;
  ImGui::BeginChild(1, ImVec2(10 * zoom, 0));
  drawlist->AddText(ImVec2(cpos.x, cpos.y - (14 * zoom)), 0xffffffff, text);
  ImGui::EndChild();
  ImGui::GetIO().FontGlobalScale = fontscale;
  ImGui::SameLine();
  ImGui::BeginChild(2, ImVec2(1, 1));
  ImGui::EndChild();
}

extern const Template templates[N_COUNT]{
    Template{.headerColor = ImNodedit::imColor(0x333A3DFF),
             .size = ImVec2(120, 60),
             .inputCount = 1,
             .outputCount = 1,
             .inputNames = N_FORWARDER__inNames,
             .outputNames = N_FORWARDER__outNames,
             .name = "przekaźnik",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), "%.0f , %.0f",
                                      node->position.x / 10,
                                      node->position.y / 10);
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

                   TextSized(drawlist, nodedit->getZoomFactor(), "#");
                   ImGui::SameLine();

                   ImGui::SetNextItemWidth(ImGui::GetWindowSize().x -
                                           img_size.x);
                   ImGui::DragFloat("##number",
                                    &((nNumber *)node->userData)->value, 0.1);
                 }},

    Template{.headerColor = ImNodedit::imColor(0xFF7F00FF),
             .size = ImVec2(120, 60),
             .inputCount = 0,
             .outputCount = 1,
             // // .inputNames = N_FORWARDER__inNames,
             // .inputColors = N_aritm_op__inColors,
             // // .outputNames = N_STRING__outNames,
             // .outputColors = N_aritm_op__outColors,
             .name = "ciąg znaków",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   ImGui::SetNextItemWidth(ImGui::GetWindowSize().x);
                   ImGui::InputText("##string",
                                    &((nString *)node->userData)->value);
                 }},

    Template{.headerColor = ImNodedit::imColor(0x51A019FF),
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             // .inputNames = N_FORWARDER__inNames,
             .inputColors = N_aritm_op__inColors,
             // .outputNames = N_STRING__outNames,
             .outputColors = N_aritm_op__outColors,
             .name = "dodaj",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   TextSized(drawlist, nodedit->getZoomFactor(), "+");
                   ImGui::SameLine();

                   auto *data = (nAdd *)node->userData;
                   if (data->inputs[1] != Link{-1, -1})
                     ImGui::BeginDisabled();

                   ImGui::SetNextItemWidth(ImGui::GetWindowSize().x -
                                           (10 * nodedit->getZoomFactor()));
                   ImGui::DragFloat("##number", &data->value, 0.1);

                   if (data->inputs[1] != Link{-1, -1})
                     ImGui::EndDisabled();
                 }},

    Template{.headerColor = ImNodedit::imColor(0x51A019FF),
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             // .inputNames = N_FORWARDER__inNames,
             .inputColors = N_aritm_op__inColors,
             // .outputNames = N_STRING__outNames,
             .outputColors = N_aritm_op__outColors,
             .name = "odejmij",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   TextSized(drawlist, nodedit->getZoomFactor(), "-");
                   ImGui::SameLine();

                   auto *data = (nSub *)node->userData;
                   if (data->inputs[1] != Link{-1, -1})
                     ImGui::BeginDisabled();

                   ImGui::SetNextItemWidth(ImGui::GetWindowSize().x -
                                           (20 * nodedit->getZoomFactor()));
                   ImGui::DragFloat("##number", &data->value, 0.1);

                   if (data->inputs[1] != Link{-1, -1})
                     ImGui::EndDisabled();
                 }},

    Template{.headerColor = ImNodedit::imColor(0x51A019FF),
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             // .inputNames = N_FORWARDER__inNames,
             .inputColors = N_aritm_op__inColors,
             // .outputNames = N_STRING__outNames,
             .outputColors = N_aritm_op__outColors,
             .name = "pomnóż",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   TextSized(drawlist, nodedit->getZoomFactor(), "*");
                   ImGui::SameLine();

                   auto *data = (nMul *)node->userData;
                   if (data->inputs[1] != Link{-1, -1})
                     ImGui::BeginDisabled();

                   ImGui::SetNextItemWidth(ImGui::GetWindowSize().x -
                                           (20 * nodedit->getZoomFactor()));
                   ImGui::DragFloat("##number", &data->value, 0.1);

                   if (data->inputs[1] != Link{-1, -1})
                     ImGui::EndDisabled();
                 }},

    Template{.headerColor = ImNodedit::imColor(0x51A019FF),
             .size = ImVec2(120, 60),
             .inputCount = 2,
             .outputCount = 1,
             // .inputNames = N_FORWARDER__inNames,
             .inputColors = N_aritm_op__inColors,
             // .outputNames = N_STRING__outNames,
             .outputColors = N_aritm_op__outColors,
             .name = "podziel",

             .draw_function =
                 [](int32_t nodeIdx, ImNodedit::Node *node,
                    ImNodedit::ImNodeEditor *nodedit, ImDrawList *drawlist) {
                   TextSized(drawlist, nodedit->getZoomFactor(), "/");
                   ImGui::SameLine();

                   auto *data = (nDiv *)node->userData;
                   if (data->inputs[1] != Link{-1, -1})
                     ImGui::BeginDisabled();

                   ImGui::SetNextItemWidth(ImGui::GetWindowSize().x -
                                           (20 * nodedit->getZoomFactor()));
                   ImGui::DragFloat("##number", &data->value, 0.1);

                   if (data->inputs[1] != Link{-1, -1})
                     ImGui::EndDisabled();
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
