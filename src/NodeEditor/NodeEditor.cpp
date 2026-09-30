#include "imgui.h"
#include <NodeEditor.hpp>
#include <vector>

struct nNode {
  bool selected = false;
  bool hovered = false;
  int32_t templateIdx = -1;
  ImVec2 position = ImVec2(0, 0);
  ImVec2 size = ImVec2(100, 20);
  const char *name;

  Link inputs[2];
  std::vector<Link> outputs[3];
};

class Nodedit : public ImNodeEditor {
protected:
  std::vector<nNode> nodes{
      nNode{.size = ImVec2(300, 100), .name = "node 1"},
      nNode{.position = ImVec2(-100, 0),
            .size = ImVec2(100, 300),
            .name = "node 2"},
      nNode{.position = ImVec2(-200, -400),
            .size = ImVec2(200, 400),
            .name = "node 3"},
  };

public:
  Nodedit() = default;
  ~Nodedit() override = default;

  void selectNode(int32_t nodeIdx, bool selected) override {
    nodes[nodeIdx].selected = selected;
  }

  void moveSelectedNodes(const ImVec2 delta) override {
    for (auto &node : nodes) {
      if (node.selected) {
        node.position.x += delta.x;
        node.position.y += delta.y;
      }
    }
  }

  int32_t getTemplateCount() override { return 1; }
  const Template getTemplate(int32_t templateIdx) {
    return Template{.inputCount = 2, .outputCount = 3};
  }

  int32_t getNodeCount() override { return nodes.size(); }
  Node getNode(int32_t nodeIdx) override {
    nNode &node = nodes[nodeIdx];
    return Node{
        .selected = node.selected,
        .hovered = node.hovered,
        .templateIdx = 0,
        .position = node.position,
        .size = node.size,
        .userData = (void *)node.name,
    };
  }

  Link getNodeInputLink(int32_t nodeIdx, int8_t pinIdx) override {
    return nodes[nodeIdx].inputs[pinIdx];
  }

  void addLink(int32_t inputNodeIdx, int8_t inputPinIdx, int32_t outputNodeIdx,
               int8_t outputPinIdx) {

    nodes[inputNodeIdx].inputs[inputPinIdx] =
        Link{.pinIdx = outputPinIdx, .nodeIdx = outputNodeIdx};

    nodes[outputNodeIdx].outputs[outputPinIdx].push_back(
        Link{.pinIdx = inputPinIdx, .nodeIdx = inputNodeIdx});
  }

  void delLink(int32_t inputNodeIdx, int8_t inputPinIdx, int32_t outputNodeIdx,
               int8_t outputPinIdx) {

    nodes[inputNodeIdx].inputs[inputPinIdx] = Link{.pinIdx = -1, .nodeIdx = -1};

    std::vector<Link> &outputs = nodes[outputNodeIdx].outputs[outputPinIdx];
    for (int i = 0; i < outputs.size(); i++) {

      if (outputs[i].pinIdx == inputPinIdx &&
          outputs[i].nodeIdx == inputNodeIdx) {

        outputs.erase(outputs.begin() + i);
        return;
      }
    }
  }

  void drawNodeWidgets(int32_t nodeIdx, Node *node) override {
    ImGui::Text("Hi I'm %s", (const char *)node->userData);
    ImGui::Button((const char *)node->userData);
  }
};

static bool in_rect(ImVec2 pos, ImVec2 min, ImVec2 max) {
  return pos.x >= min.x && pos.x <= max.x && pos.y >= min.y && pos.y <= max.y;
}

ImVec2 ImNodeEditor::world2screen(ImVec2 vec) {
  return ImVec2((vec.x + offset.x) * zoom, (vec.y + offset.y) * zoom);
}

ImVec2 ImNodeEditor::screen2world(ImVec2 vec) {
  return ImVec2((vec.x / zoom) - offset.x, (vec.y / zoom) - offset.y);
}

void ImNodeEditor::update() {
  ImDrawList *drawlist = ImGui::GetWindowDrawList();
  ImVec2 wsize = ImGui::GetWindowSize();

  zoom += ImGui::GetIO().MouseWheel * 0.1;
  zoom = zoom <= 0.1 ? 0.1 : zoom;

  if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
    ImVec2 delta = ImGui::GetIO().MouseDelta;
    offset = ImVec2(offset.x + screen2world(delta).x, offset.y + screen2world(delta).y);
  }

  ImGui::GetIO().MouseWheel;
  for (int32_t nodeIdx = 0; nodeIdx < getNodeCount(); nodeIdx++) {
    Node node = getNode(nodeIdx);
    Template templ = getTemplate(node.templateIdx);
    ImGui::SetCursorPos(ImVec2(world2screen(node.position).x + (wsize.x / 2),
                               world2screen(node.position).y + (wsize.y / 2)));

    ImGui::BeginChild(nodeIdx + 1,
                      ImVec2(node.size.x * zoom, node.size.y * zoom));
    {
      ImVec2 wpos = ImGui::GetWindowPos();
      ImVec2 wsize = ImVec2(node.size.x * zoom, node.size.y * zoom);
      ImVec2 wpos2 = ImVec2(wpos.x + wsize.x, wpos.y + wsize.y);
      bool hovered = in_rect(ImGui::GetMousePos(), wpos, wpos2);
      // 2C3335
      drawlist->AddRectFilled(
          wpos, wpos2, hovered ? imColor(0x2C3335FF) : imColor(0x24292BFF), 3);

      ImGui::SetCursorPos(ImVec2(10, 4));
      ImGui::TextUnformatted(templ.name);
      drawlist->AddRectFilled(
          wpos, ImVec2(wpos2.x, wpos.y + 25), templ.headerColor, 4,
          ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight);
      // drawlist->AddRect(wpos, wpos2, imColor(0x2F3538FF), 4, 0, 2);

      float in_frac =
          (wsize.y - 25) / (templ.inputCount ? templ.inputCount : 1);
      for (int i = 0; i < templ.inputCount; i++) {
        const char *name = templ.inputNames ? templ.inputNames[i] : "";
        ImU32 color = templ.inputColors ? templ.inputColors[i]
                                        : ImU32(ImColor(100, 100, 100));

        ImVec2 circle_pos =
            ImVec2(wpos.x, (wpos.y + 25) + (in_frac * (i + 0.5)));
        drawlist->AddCircleFilled(circle_pos, 5, color);
      }

      float out_frac =
          (wsize.y - 25) / (templ.outputCount ? templ.outputCount : 1);
      for (int i = 0; i < templ.outputCount; i++) {
        const char *name = templ.outputNames ? templ.outputNames[i] : "";
        ImU32 color = templ.outputColors ? templ.outputColors[i]
                                         : ImU32(ImColor(100, 100, 100));

        ImVec2 circle_pos =
            ImVec2(wpos2.x, (wpos.y + 25) + (out_frac * (i + 0.5)));
        drawlist->AddCircleFilled(circle_pos, 5, color);
      }

      ImGui::SetCursorPos(ImVec2(10, 35));
      ImGui::BeginChild(1, ImVec2(wsize.x - 20, wsize.y - 45));
      drawNodeWidgets(nodeIdx, &node);
      ImGui::EndChild();
    }
    ImGui::EndChild();
  }
}

void nodedit_update() {
  static Nodedit nodedit{};
  nodedit.update();
}
