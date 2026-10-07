#pragma once
#include <Any_and_Pool.hpp>
#include <memory>
#include <string>
#include <vector>

struct Node {
  bool selected = false;
  // bool hovered = false;
  tHandle node_data;
  int32_t templateIdx = -1;
  float position[2] = {0};
  // float size[2] = {300, 100};
  // void *userData = nullptr;

  inline void setPos(float x, float y) {
    position[0] = x;
    position[1] = y;
  }
};

struct Link {
  int8_t pinIdx = -1;
  int32_t nodeIdx = -1;

  inline bool operator==(Link other) {
    return pinIdx == other.pinIdx && nodeIdx == other.nodeIdx;
  }
};

class NodePoolBase {
public:
  virtual ~NodePoolBase() = default;
  // virtual void *expose() = 0;
  virtual void *get(tHandle handle) = 0;
  virtual void remove(tHandle handle) = 0;
  virtual NodePoolBase *copy() const = 0;
  virtual Link *inputsOf(void *node_data) = 0;

  template <typename T> T *convertTo() { return (T *)this; }
};

template <typename T> class NodePool : public NodePoolBase {
public:
  Pool<T> pool;

  ~NodePool() override = default;

  // void *expose() override { return (void *)pool.expose().data(); }

  void *get(tHandle handle) override { return (void *)pool.get(handle); }

  void remove(tHandle handle) override { pool.remove(handle); }

  NodePoolBase *copy() const override {
    NodePool<T> *p = new NodePool<T>;
    *p = *this;
    return p;
  }

  Link *inputsOf(void *node_data) override { return ((T *)node_data)->inputs; }
};

enum NodeGraph_NodeType {
  N_FORWARDER = 0,
  N_NUMBER,
  N_STRING,

  N_ADD,
  N_SUB,
  N_MUL,
  N_DIV,

  N_RETURN,
  N_COUNT,
};

class NodeGraph {
public:
  Pool<Node> nodes;
  std::vector<std::unique_ptr<NodePoolBase>> nodeData;

  NodeGraph();
  ~NodeGraph() = default;
  NodeGraph(const NodeGraph &other);
  NodeGraph &operator=(const NodeGraph &other);
  NodeGraph(NodeGraph &&other) = default;
  NodeGraph &operator=(NodeGraph &&other) = default;

  template <typename T> Node *add_node(NodeGraph_NodeType type, T &&node_data) {
    tHandle h =
        nodeData[(size_t)type].get()->convertTo<NodePool<T>>()->pool.add(
            node_data);

    tHandle n = nodes.add(Node{.node_data = h, .templateIdx = type});
    return nodes.get(n);
  }
};

struct nForwarder {
  Link inputs[1];
};

struct nNumber {
  float value;
  Link inputs[0];
};

struct nString {
  std::string value;
  Link inputs[0];
};

struct nAdd {
  float value;
  Link inputs[2];
};

struct nSub {
  float value;
  Link inputs[2];
};

struct nMul {
  float value;
  Link inputs[2];
};

struct nDiv {
  float value;
  Link inputs[2];
};

struct nReturn {
  Link inputs[1];
};
