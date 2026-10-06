#pragma once
#include <Any_and_Pool.hpp>
#include <memory>
#include <vector>

struct Node {
  bool selected = false;
  // bool hovered = false;
  tHandle node_data;
  int32_t templateIdx = -1;
  float position[2] = {0};
  float size[2] = {100, 20};
  // void *userData = nullptr;
};

struct Link {
  int8_t pinIdx = -1;
  int16_t nodeIdx = -1;
};

class NodePoolBase {
public:
  virtual ~NodePoolBase() = default;
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

  void *get(tHandle handle) override { return (void *)pool.get(handle); }

  void remove(tHandle handle) override { pool.remove(handle); }

  NodePoolBase *copy() const override {
    NodePool<T> *p = new NodePool<T>;
    *p = *this;
    return p;
  }

  Link *inputsOf(void *node_data) override { return ((T *)node_data)->inputs; }
};

struct nForwarder {
  Link inputs[1] = {0};
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
};
