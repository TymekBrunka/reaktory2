#include "imgui.h"
#include <NodeEditor.hpp>
#include <iostream>
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
      nNode{.position = ImVec2(-100, 0),
            .size = ImVec2(100, 300),
            .name = "node 1"},
      nNode{
          .size = ImVec2(300, 100),
          .name = "node 2",
          .outputs = {{{0, 2}}, {}, {}},
      },
      nNode{
          .position = ImVec2(-200, -400),
          .size = ImVec2(200, 400),
          .name = "node 3",
          .inputs = {{0, 1}, {-1, -1}},
      },
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
        .position = ImVec2(((int)node.position.x / 10) * 10,
                           (((int)node.position.y) / 10) * 10),
        .size = node.size,
        .userData = (void *)node.name,
    };
  }

  Link getNodeInputLink(int32_t nodeIdx, int8_t pinIdx) override {
    return nodes[nodeIdx].inputs[pinIdx];
  }

  void addLink(int32_t inputNodeIdx, int8_t inputPinIdx, int32_t outputNodeIdx,
               int8_t outputPinIdx) {

    std::cerr << "linking {" << inputNodeIdx << "," << (int)inputPinIdx
              << "} and {" << outputNodeIdx << "," << (int)outputPinIdx
              << "}\n";

    nodes[inputNodeIdx].inputs[inputPinIdx] =
        Link{.pinIdx = outputPinIdx, .nodeIdx = outputNodeIdx};

    nodes[outputNodeIdx].outputs[outputPinIdx].push_back(
        Link{.pinIdx = inputPinIdx, .nodeIdx = inputNodeIdx});
  }

  void delLink(int32_t inputNodeIdx, int8_t inputPinIdx, int32_t outputNodeIdx,
               int8_t outputPinIdx) {

    std::cerr << "UNlinking {" << inputNodeIdx << "," << (int)inputPinIdx
              << "} and {" << outputNodeIdx << "," << (int)outputPinIdx
              << "}\n";

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

  void drawNodeWidgets(int32_t nodeIdx, Node *node,
                       ImNodeEditor *nodedit) override {

    ImGui::Text("Hi I'm %s", (const char *)node->userData);
    ImGui::Button((const char *)node->userData);
  }
};

static ImVec2 negative_vector(ImVec2 vec) { return ImVec2(-vec.x, -vec.y); }

static ImVec2 add_vector(ImVec2 a, ImVec2 b) {
  return ImVec2(a.x + b.x, a.y + b.y);
}

static ImVec2 sub_vector(ImVec2 a, ImVec2 b) {
  return ImVec2(a.x - b.x, a.y - b.y);
}

static ImVec2 scale_vector(ImVec2 a, float scale) {
  return ImVec2(a.x * scale, a.y * scale);
}

static bool in_rect(ImVec2 pos, ImVec2 min, ImVec2 max) {
  return pos.x >= min.x && pos.x <= max.x && pos.y >= min.y && pos.y <= max.y;
}

static float vec_length_squered(ImVec2 a, ImVec2 b) {
  return ((b.x - a.x) * (b.x - a.x)) + ((b.y - a.y) * (b.y - a.y));
}

static bool rect_in_rect(ImVec2 a1, ImVec2 a2, ImVec2 b1, ImVec2 b2) {
  return in_rect(a1, b1, b2) || in_rect(ImVec2(a1.x, a2.y), b1, b2) ||
         in_rect(ImVec2(a2.x, a2.y), b1, b2) ||
         in_rect(ImVec2(a2.x, a1.y), b1, b2);
}

ImVec2 ImNodeEditor::world2screen(ImVec2 vec) {
  return add_vector(add_vector(scale_vector(add_vector(vec, offset), zoom),
                               scale_vector(global_wsize, 0.5)),
                    global_wpos);
}

ImVec2 ImNodeEditor::screen2world(ImVec2 vec) {
  return sub_vector(sub_vector(sub_vector(scale_vector(vec, 1.0 / zoom),
                                          scale_vector(global_wsize, 0.5)),
                               global_wpos),
                    offset);
}

ImVec2 ImNodeEditor::getInputPinPos(const Node &node, const Template &templ,
                                    int8_t pinIdx) const {

  float frac = (node.size.y - 25) / templ.inputCount;
  return ImVec2(node.position.x,
                node.position.y + 25 + (frac * (pinIdx + 0.5)));
}

ImVec2 ImNodeEditor::getOutputPinPos(const Node &node, const Template &templ,
                                     int8_t pinIdx) const {

  float frac = (node.size.y - 25) / templ.outputCount;
  return ImVec2(node.position.x + node.size.x,
                node.position.y + 25 + (frac * (pinIdx + 0.5)));
}

void ImNodeEditor::update() {
  bool link_is_being_dropped = false;

  global_wpos = ImGui::GetWindowPos();
  global_wsize = ImGui::GetWindowSize();
  ImDrawList *drawlist = ImGui::GetWindowDrawList();
  float original_font_scale = ImGui::GetIO().FontGlobalScale;
  ImGui::GetIO().FontGlobalScale = zoom;

  if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered()) {

    zoom += ImGui::GetIO().MouseWheel * 0.1;
    zoom = zoom <= 0.1 ? 0.1 : zoom;

    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
      ImVec2 delta = ImGui::GetIO().MouseDelta;
      // offset = ImVec2(offset.x + (delta.x / zoom), offset.y + (delta.y /
      // zoom));
      offset = add_vector(offset, scale_vector(delta, 1.0 / zoom));
    }
  }

  bool click = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
  if (ImGui::IsWindowFocused() ||
      ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {

    link_is_being_dropped = !ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                            currently_dragged_pin.pinIdx != -1 &&
                            currently_dragged_pin.nodeIdx != -1;

    if (click && !ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
      for (int32_t i = 0; i < getNodeCount(); i++) {
        selectNode(i, false);
      }
    }
  }

  // draw 1st layer and handle user input
  for (int32_t nodeIdx = 0; nodeIdx < getNodeCount(); nodeIdx++) {
    Node node = getNode(nodeIdx);
    Template templ = getTemplate(node.templateIdx);
    // draw links (from input to output)
    for (int8_t i = 0; i < templ.inputCount; i++) {
      ImVec2 circle_pos = world2screen(getInputPinPos(node, templ, i));
      Link link = getNodeInputLink(nodeIdx, i);

      // draw link
      if (link.nodeIdx != -1 && link.pinIdx != -1) {
        Node node2 = getNode(link.nodeIdx);

        ImVec2 output_pin_pos = world2screen(getOutputPinPos(
            node2, getTemplate(node2.templateIdx), link.pinIdx));

        drawlist->AddLine(circle_pos, output_pin_pos, ImColor(100, 100, 100),
                          6 * zoom);
      }

      // handle input only for on-screen nodes
      if (!rect_in_rect(world2screen(node.position),
                        world2screen(add_vector(node.position, node.size)),
                        global_wpos, add_vector(global_wpos, global_wsize)))
        continue;

      // when hovering over input pin
      if (vec_length_squered(ImGui::GetMousePos(), world2screen(getInputPinPos(
                                                       node, templ, i))) < 36) {

        if (click) {
          currently_dragged_pin = {.pinIdx = i, .nodeIdx = nodeIdx};
          is_it_output_pin = false;
        }

        // when input pin is getting linked
        else if (link_is_being_dropped && is_it_output_pin &&
                 currently_dragged_pin.pinIdx != -1 &&
                 currently_dragged_pin.nodeIdx != -1) {

          link_is_being_dropped = false;
          Link current_link = getNodeInputLink(nodeIdx, i);
          if (current_link.nodeIdx != -1 || current_link.pinIdx != -1)
            delLink(nodeIdx, i, currently_dragged_pin.nodeIdx,
                    currently_dragged_pin.pinIdx);

          addLink(nodeIdx, i, currently_dragged_pin.nodeIdx,
                  currently_dragged_pin.pinIdx);

          currently_dragged_pin = {-1, -1};
        }
      }
    }

    // handle input only for on-screen nodes
    if (!rect_in_rect(world2screen(node.position),
                      world2screen(add_vector(node.position, node.size)),
                      global_wpos, add_vector(global_wpos, global_wsize)))
      continue;

    for (int8_t i = 0; i < templ.outputCount; i++) {
      // when hovering over output pin
      if (vec_length_squered(ImGui::GetMousePos(), world2screen(getOutputPinPos(
                                                       node, templ, i))) < 36) {
        if (click) {
          currently_dragged_pin = {.pinIdx = i, .nodeIdx = nodeIdx};
          is_it_output_pin = true;
        }

        // when output pin is getting linked
        else if (link_is_being_dropped && !is_it_output_pin &&
                 currently_dragged_pin.pinIdx != -1 &&
                 currently_dragged_pin.nodeIdx != -1) {

          link_is_being_dropped = false;
          Link old_link = getNodeInputLink(currently_dragged_pin.nodeIdx,
                                           currently_dragged_pin.pinIdx);
          if (old_link.pinIdx != -1 && old_link.nodeIdx != -1)
            delLink(currently_dragged_pin.nodeIdx, currently_dragged_pin.pinIdx,
                    old_link.nodeIdx, old_link.pinIdx);
          addLink(currently_dragged_pin.nodeIdx, currently_dragged_pin.pinIdx,
                  nodeIdx, i);
          currently_dragged_pin = {-1, -1};
        }
      }
    }

    // is node clicked (by header bar)
    if (click && in_rect(ImGui::GetMousePos(), world2screen(node.position),
                         world2screen(add_vector(node.position,
                                                 ImVec2(node.size.x, 25))))) {

      selectNode(nodeIdx, true);
    }
  }

  if ((ImGui::IsWindowFocused() ||
       ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) &&
      !click && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    moveSelectedNodes(scale_vector(ImGui::GetIO().MouseDelta, 1.0 / zoom));
  }

  if (currently_dragged_pin.pinIdx != -1 &&
      currently_dragged_pin.nodeIdx != -1) {

    Node node = getNode(currently_dragged_pin.nodeIdx);
    ImVec2 pinpos = is_it_output_pin
                        ? getOutputPinPos(node, getTemplate(node.templateIdx),
                                          currently_dragged_pin.pinIdx)
                        : getInputPinPos(node, getTemplate(node.templateIdx),
                                         currently_dragged_pin.pinIdx);

    pinpos = world2screen(pinpos);
    drawlist->AddLine(pinpos, ImGui::GetMousePos(),
                      ImU32(ImColor(200, 200, 200)), 6 * zoom);
  }

  // 2nd pass, only for drawing
  for (int32_t nodeIdx = 0; nodeIdx < getNodeCount(); nodeIdx++) {
    Node node = getNode(nodeIdx);
    Template templ = getTemplate(node.templateIdx);

    ImVec2 wpos = world2screen(node.position);
    ImVec2 wsize = scale_vector(node.size, zoom);
    ImVec2 wpos2 = add_vector(wpos, wsize);
    bool hovered = in_rect(ImGui::GetMousePos(), wpos, wpos2);
    drawlist->AddRectFilled(wpos, wpos2,
                            hovered ? imColor(0x2C3335FF) : imColor(0x24292BFF),
                            4 * zoom);

    drawlist->AddRectFilled(wpos, ImVec2(wpos2.x, wpos.y + (25 * zoom)),
                            templ.headerColor, 4 * zoom,
                            ImDrawFlags_RoundCornersTopLeft |
                                ImDrawFlags_RoundCornersTopRight);

    drawlist->AddText(add_vector(wpos, scale_vector(ImVec2(10, 4), zoom)),
                      imColor(0xffffffff), templ.name);

    if (node.selected)
      drawlist->AddRect(wpos, wpos2, imColor(0xffaa00ff), 4 * zoom, 0, 2);

    // draw input pins
    {
      for (int8_t i = 0; i < templ.inputCount; i++) {
        const char *name = templ.inputNames ? templ.inputNames[i] : "";
        ImU32 color = templ.inputColors ? templ.inputColors[i]
                                        : ImU32(ImColor(100, 100, 100));

        ImVec2 circle_pos = world2screen(getInputPinPos(node, templ, i));

        bool hovered = vec_length_squered(ImGui::GetMousePos(), circle_pos) <
                       36 * zoom * zoom;
        drawlist->AddCircleFilled(circle_pos, 6 * zoom,
                                  hovered ? ImU32(ImColor(200, 200, 200))
                                          : color);
      }
    }
    // draw output pins
    {
      float out_frac =
          (wsize.y - (25 * zoom)) / (templ.outputCount ? templ.outputCount : 1);
      for (int8_t i = 0; i < templ.outputCount; i++) {
        const char *name = templ.outputNames ? templ.outputNames[i] : "";
        ImU32 color = templ.outputColors ? templ.outputColors[i]
                                         : ImU32(ImColor(100, 100, 100));

        // ImVec2 circle_pos =
        //     ImVec2(wpos2.x, (wpos.y + (25 * zoom)) + (out_frac * (i +
        //     0.5)));
        ImVec2 circle_pos = world2screen(getOutputPinPos(node, templ, i));

        bool hovered = vec_length_squered(ImGui::GetMousePos(), circle_pos) <
                       36 * zoom * zoom;
        drawlist->AddCircleFilled(circle_pos, 6 * zoom,
                                  hovered ? ImU32(ImColor(200, 200, 200))
                                          : color);
      }
    }

    ImGui::SetNextWindowPos(
        add_vector(wpos, scale_vector(ImVec2(10, 35), zoom)));
    ImGui::BeginChild(
        nodeIdx + 1, add_vector(wsize, ImVec2(-20 * zoom, -45 * zoom)),
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    drawNodeWidgets(nodeIdx, &node, this);
    ImGui::EndChild();
  }

  if (link_is_being_dropped)
    currently_dragged_pin = {-1, -1};

  ImGui::GetIO().FontGlobalScale = original_font_scale;
}

void nodedit_update() {
  static Nodedit nodedit{};
  nodedit.update();
}
