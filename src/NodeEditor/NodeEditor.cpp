#include "imgui.h"
#include <NodeEditor.hpp>
#include <cmath>
#include <cstdint>
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
    static ImU32 inColors[] = {
        ImColor(200, 100, 0),
        ImColor(200, 0, 200),
    };

    static ImU32 outColors[] = {
        ImColor(200, 0, 0),
        ImColor(0, 200, 0),
        ImColor(0, 0, 200),
    };

    return Template{.inputCount = 2,
                    .outputCount = 3,
                    .inputColors = inColors,
                    .outputColors = outColors};
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

static float sdf_line_squered(ImVec2 p, ImVec2 a, ImVec2 b) {
  ImVec2 pa = sub_vector(p, a);
  ImVec2 ba = sub_vector(b, a);
  float h =
      ((pa.x * ba.x) + (pa.y * ba.y)) / vec_length_squered(ImVec2(0, 0), ba);
  h = h < 0 ? 0 : (h > 1 ? 1 : h);
  return vec_length_squered(scale_vector(ba, h), pa);
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

struct RgbColor {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct HsvColor {
  unsigned char h;
  unsigned char s;
  unsigned char v;
};

static RgbColor HsvToRgb(HsvColor hsv) {
  RgbColor rgb;
  unsigned char region, p, q, t;
  unsigned int h, s, v, remainder;

  if (hsv.s == 0) {
    rgb.r = hsv.v;
    rgb.g = hsv.v;
    rgb.b = hsv.v;
    return rgb;
  }

  // converting to 16 bit to prevent overflow
  h = hsv.h;
  s = hsv.s;
  v = hsv.v;

  region = h / 43;
  remainder = (h - (region * 43)) * 6;

  p = (v * (255 - s)) >> 8;
  q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

  switch (region) {
  case 0:
    rgb.r = v;
    rgb.g = t;
    rgb.b = p;
    break;
  case 1:
    rgb.r = q;
    rgb.g = v;
    rgb.b = p;
    break;
  case 2:
    rgb.r = p;
    rgb.g = v;
    rgb.b = t;
    break;
  case 3:
    rgb.r = p;
    rgb.g = q;
    rgb.b = v;
    break;
  case 4:
    rgb.r = t;
    rgb.g = p;
    rgb.b = v;
    break;
  default:
    rgb.r = v;
    rgb.g = p;
    rgb.b = q;
    break;
  }

  return rgb;
}

static HsvColor RgbToHsv(RgbColor rgb) {
  HsvColor hsv;
  unsigned char rgbMin, rgbMax;

  rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b)
                         : (rgb.g < rgb.b ? rgb.g : rgb.b);
  rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b)
                         : (rgb.g > rgb.b ? rgb.g : rgb.b);

  hsv.v = rgbMax;
  if (hsv.v == 0) {
    hsv.h = 0;
    hsv.s = 0;
    return hsv;
  }

  hsv.s = 255 * ((long)(rgbMax - rgbMin)) / hsv.v;
  if (hsv.s == 0) {
    hsv.h = 0;
    return hsv;
  }

  if (rgbMax == rgb.r)
    hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
  else if (rgbMax == rgb.g)
    hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
  else
    hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

  return hsv;
}

inline ImU32 imColorBrighten(ImU32 x, float factor) {
  uint8_t *in = (uint8_t *)&x;
  RgbColor rgb{.r = in[2], .g = in[1], .b = in[0]};
  HsvColor color = RgbToHsv(rgb);

  std::cerr << "color is " << (int)color.h << "," << (int)color.s << ","
            << (int)color.v << "\n";
  // color.v = color.v + factor > 1 ? 1 : color.v + factor;
  float fac = 0.5;
  // float frac = (float)color.v / (float)color.s;
  float s = (float)color.s;
  float sat = sat - (fac * 2.5 * sqrt(s));
  float val = (float)color.v + (fac * 1 / (s ? s : 0.1));
  // float val = (float)color.v + fac;
  color.s = sat < 0 ? 0 : color.s - (uint8_t)sat;
  // color.v = val > 255 ? 255 : color.v + (uint8_t)val;

  // color.v += 20;

  // color.v = color.v > 1 ? 1 : color.v;

  rgb = HsvToRgb(color);
  uint8_t out[4] = {rgb.b, rgb.g, rgb.r, in[3]};
  // uint8_t out[4] = {in[0], rgb.b, rgb.g, rgb.r};
  return *(ImU32 *)&out;
}

void ImNodeEditor::update() {
  bool link_is_being_dropped = false;
  bool has_output_pin_been_clicked = false;

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
      currently_selected_link[0] = {-1, -1};
      currently_selected_link[1] = {-1, -1};
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

      // draw link and handle user input to it
      if (link.nodeIdx != -1 && link.pinIdx != -1) {
        if (vec_length_squered(ImGui::GetMousePos(), circle_pos) <=
                100 * zoom * zoom &&
            ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          delLink(nodeIdx, i, link.nodeIdx, link.pinIdx);
          currently_selected_link[0] = {-1, -1};
          currently_selected_link[1] = {-1, -1};
          continue;
        }
        Node node2 = getNode(link.nodeIdx);
        Template templ = getTemplate(node2.templateIdx);

        ImU32 color = templ.outputColors ? templ.outputColors[link.pinIdx]
                                         : ImU32(ImColor(100, 100, 100));

        ImVec2 output_pin_pos = world2screen(getOutputPinPos(
            node2, getTemplate(node2.templateIdx), link.pinIdx));

        bool selected = currently_selected_link[0].pinIdx == i &&
                        currently_selected_link[0].nodeIdx == nodeIdx &&
                        currently_selected_link[1].pinIdx == link.pinIdx &&
                        currently_selected_link[1].nodeIdx == link.nodeIdx;

        bool hovered = sdf_line_squered(ImGui::GetMousePos(), circle_pos,
                                        output_pin_pos) <= 36 * zoom * zoom;

        if ((hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) ||
            (selected && ImGui::IsKeyPressed(ImGuiKey_Delete))) {

          delLink(nodeIdx, i, link.nodeIdx, link.pinIdx);
          currently_selected_link[0] = {-1, -1};
          currently_selected_link[1] = {-1, -1};
        } else if (hovered && click && !has_output_pin_been_clicked) {
          currently_selected_link[0] = {.pinIdx = i, .nodeIdx = nodeIdx};
          currently_selected_link[1] = link;
        }

        drawlist->AddLine(
            circle_pos, output_pin_pos,
            selected ? imColor(0xffaa00ff)
                     : (hovered ? ImU32(ImColor(200, 200, 200)) : color),
            6 * zoom);
      }

      // handle input only for on-screen nodes
      if (!rect_in_rect(world2screen(node.position),
                        world2screen(add_vector(node.position, node.size)),
                        global_wpos, add_vector(global_wpos, global_wsize)))
        continue;

      // when hovering over input pin
      if (vec_length_squered(ImGui::GetMousePos(),
                             world2screen(getInputPinPos(node, templ, i))) <
          100 * zoom * zoom) {

        if (click) {
          currently_dragged_pin = {.pinIdx = i, .nodeIdx = nodeIdx};
          currently_selected_link[0] = {-1, -1};
          currently_selected_link[1] = {-1, -1};
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
      if (vec_length_squered(ImGui::GetMousePos(),
                             world2screen(getOutputPinPos(node, templ, i))) <
          100 * zoom * zoom) {
        if (click) {
          currently_dragged_pin = {.pinIdx = i, .nodeIdx = nodeIdx};
          currently_selected_link[0] = {-1, -1};
          currently_selected_link[1] = {-1, -1};
          is_it_output_pin = true;
          has_output_pin_been_clicked =
              true; // only in this specific scenario, link migh be still
                    // selected when dragging new one
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
      currently_selected_link[0] = {-1, -1};
      currently_selected_link[1] = {-1, -1};
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
    Template templ = getTemplate(node.templateIdx);
    ImVec2 pinpos = is_it_output_pin
                        ? getOutputPinPos(node, getTemplate(node.templateIdx),
                                          currently_dragged_pin.pinIdx)
                        : getInputPinPos(node, getTemplate(node.templateIdx),
                                         currently_dragged_pin.pinIdx);

    ImU32 color = is_it_output_pin
                      ? (templ.outputColors
                             ? templ.outputColors[currently_dragged_pin.pinIdx]
                             : ImU32(ImColor(100, 100, 100)))
                      : (templ.inputColors
                             ? templ.inputColors[currently_dragged_pin.pinIdx]
                             : ImU32(ImColor(100, 100, 100)));

    pinpos = world2screen(pinpos);
    drawlist->AddLine(pinpos, ImGui::GetMousePos(), color, 6 * zoom);
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
                            hovered
                                ? imColorBrighten(templ.backgroundColor, 0.1)
                                : templ.backgroundColor,
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

        bool dragged = currently_dragged_pin.pinIdx == i &&
                       currently_dragged_pin.nodeIdx == nodeIdx &&
                       !is_it_output_pin;

        float mdist = vec_length_squered(ImGui::GetMousePos(), circle_pos);

        drawlist->AddCircleFilled(circle_pos, 6 * zoom,
                                  mdist <= 36 * zoom * zoom || dragged
                                      ? imColorBrighten(color, 0.1)
                                      : color);

        if (node.selected)
          drawlist->AddCircle(circle_pos, 6 * zoom, imColor(0xffaa00ff), 16, 2);

        if (mdist <= 64 && currently_dragged_pin.pinIdx != -1 &&
            currently_dragged_pin.nodeIdx != -1 && is_it_output_pin) {
          Node node2 = getNode(currently_dragged_pin.nodeIdx);
          Template templ = getTemplate(node2.templateIdx);
          ImU32 color = templ.outputColors
                            ? templ.outputColors[currently_dragged_pin.pinIdx]
                            : ImU32(ImColor(100, 100, 100));

          drawlist->AddCircle(circle_pos, 10 * zoom, ImColor(200, 200, 200), 16,
                              2);
        }
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

        bool dragged = currently_dragged_pin.pinIdx == i &&
                       currently_dragged_pin.nodeIdx == nodeIdx &&
                       is_it_output_pin;

        float mdist = vec_length_squered(ImGui::GetMousePos(), circle_pos);

        drawlist->AddCircleFilled(circle_pos, 6 * zoom,
                                  mdist <= 36 * zoom * zoom || dragged
                                      ? imColorBrighten(color, 0.1)
                                      : color);

        if (node.selected)
          drawlist->AddCircle(circle_pos, 6 * zoom, imColor(0xffaa00ff), 16, 2);

        if (mdist <= 64 && currently_dragged_pin.pinIdx != -1 &&
            currently_dragged_pin.nodeIdx != -1 && !is_it_output_pin) {
          Node node2 = getNode(currently_dragged_pin.nodeIdx);
          Template templ = getTemplate(node2.templateIdx);
          ImU32 color = templ.inputColors
                            ? templ.inputColors[currently_dragged_pin.pinIdx]
                            : ImU32(ImColor(100, 100, 100));

          drawlist->AddCircle(circle_pos, 10 * zoom, color, 16, 2);
        }
      }
    }

    ImGui::SetNextWindowPos(
        add_vector(wpos, scale_vector(ImVec2(10, 35), zoom)));
    ImGui::BeginChild(nodeIdx + 1,
                      add_vector(wsize, ImVec2(-20 * zoom, -45 * zoom)));
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
