#pragma once
#include <Any_and_Pool.hpp>
#include <memory>
#include <string>
#include <vector>

struct Node {
  bool selected = false;
  tHandle node_data;
  int32_t templateIdx = -1;
  float position[2] = {0};
  void *userData = nullptr;

  inline void setPos(float x, float y) {
    position[0] = x;
    position[1] = y;
  }
};

struct Link {
  int8_t pinIdx = -1;
  int16_t nodeIdx = -1;

  inline bool operator==(Link other) {
    return pinIdx == other.pinIdx && nodeIdx == other.nodeIdx;
  }
};

class NodePoolBase {
public:
  virtual ~NodePoolBase() = default;
  // virtual void *expose() = 0;
  virtual void *get(tHandle handle, bool unsafe = false) = 0;
  virtual tHandle add() = 0;
  virtual void remove(tHandle handle, bool unsafe = false) = 0;
  virtual NodePoolBase *copy() const = 0;
  virtual Link *inputsOf(void *node_data) = 0;
  virtual std::vector<Link> *outputsOf(void *node_data) = 0;

  template <typename T> T *convertTo() { return (T *)this; }
};

template <typename T> class NodePool : public NodePoolBase {
public:
  Pool<T> pool;

  ~NodePool() override = default;

  // void *expose() override { return (void *)pool.expose().data(); }

  void *get(tHandle handle, bool unsafe = false) override {
    return (void *)pool.get(handle, unsafe);
  }

  tHandle add() override {
    tHandle h = pool.add(std::move(T{}));
    return h;
  }

  void remove(tHandle handle, bool unsafe = false) override {
    pool.remove(handle, unsafe);
  }

  NodePoolBase *copy() const override {
    NodePool<T> *p = new NodePool<T>;
    *p = *this;
    return p;
  }

  Link *inputsOf(void *node_data) override { return ((T *)node_data)->inputs; }
  std::vector<Link> *outputsOf(void *node_data) override {
    return ((T *)node_data)->outputs;
  }
};

enum NodeGraph_NodeType {
  N_FORWARDER = 0,
  N_BOOL,
  N_NUMBER,
  N_STRING,
  N_VARIABLE,

  N_ADD,
  N_SUB,
  N_MUL,
  N_DIV,
  N_SET,

  N_EQUAL,
  N_LESS,
  N_MORE,
  N_LESS_OR_EQUAL,
  N_MORE_OR_EQUAL,

  N_NOT,
  N_IF,
  N_LOOP,

  N_RETURN,
  N_START,
  N_COUNT,
};

class NodeGraph {
private:
  Any recursiveDescend(Link olink);

public:
  Pool<Node> nodes;
  std::vector<std::unique_ptr<NodePoolBase>> nodeData;

  NodeGraph();
  ~NodeGraph() = default;
  NodeGraph(const NodeGraph &other);
  NodeGraph &operator=(const NodeGraph &other);
  NodeGraph(NodeGraph &&other) = default;
  NodeGraph &operator=(NodeGraph &&other) = default;

  inline Node *add_node(NodeGraph_NodeType type) {
    tHandle h = nodeData[(size_t)type].get()->add();

    tHandle n = nodes.add(Node{.node_data = h, .templateIdx = type});
    return nodes.get(n);
  }

  template <typename T> Node *add_node(NodeGraph_NodeType type, T &&node_data) {
    tHandle h =
        nodeData[(size_t)type].get()->convertTo<NodePool<T>>()->pool.add(
            std::move(node_data));

    tHandle n = nodes.add(Node{.node_data = h, .templateIdx = type});
    return nodes.get(n);
  }

  bool calculate();
};

struct nForwarder {
  Link inputs[1];
  std::vector<Link> outputs[1];
};

struct nBool {
  Any value{true};
  std::vector<Link> outputs[1];
  Link inputs[0];
};

struct nNumber {
  Any value{1.0f};
  std::vector<Link> outputs[1];
  Link inputs[0];
};

struct nString {
  Any value{std::string("")};
  std::vector<Link> outputs[1];
  Link inputs[0];
};

struct nVariable {
  Any value;
  std::string name;
  std::vector<Link> outputs[1];
  Link inputs[0];
};

struct nAdd {
  Any value{1.0f};
  Link inputs[2];
  std::vector<Link> outputs[1];
};

struct nSub {
  Any value{1.0f};
  Link inputs[2];
  std::vector<Link> outputs[1];
};

struct nMul {
  Any value{2.0f};
  Link inputs[2];
  std::vector<Link> outputs[1];
};

struct nDiv {
  Any value{2.0f};
  Link inputs[2];
  std::vector<Link> outputs[1];
};

struct nSet {
  Any value{2.0f};
  Link inputs[3];
  std::vector<Link> outputs[2];
};

struct nEqual {
  Any value{0.0f};
  Link inputs[2];
  std::vector<Link> outputs[1];
};

struct nLess {
  Any value{0.0f};
  Link inputs[2];
  std::vector<Link> outputs[1];
};

struct nMore {
  Any value{0.0f};
  Link inputs[2];
  std::vector<Link> outputs[1];
};

struct nLessOrEqual {
  Any value{0.0f};
  Link inputs[2];
  std::vector<Link> outputs[1];
};

struct nMoreOrEqual {
  Any value{0.0f};
  Link inputs[2];
  std::vector<Link> outputs[1];
};

struct nNot {
  Any value;
  Link inputs[1];
  std::vector<Link> outputs[1];
};

struct nIf {
  Any value;
  Link inputs[2];
  std::vector<Link> outputs[3];
};

struct nLoop {
  Any value;
  Link inputs[4];
  std::vector<Link> outputs[3];
};

struct nReturn {
  Any value{2137.0f};
  Link inputs[2];
  std::vector<Link> outputs[0];
};

struct nStart {
  std::vector<Link> outputs[1];
  Link inputs[0];
};
