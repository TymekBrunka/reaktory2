#include <Formulator.hpp>

NodeGraph::NodeGraph(const NodeGraph &other) {
  nodes = other.nodes;
  nodeData.clear();
  for (auto &node : other.nodeData) {
    nodeData.push_back(std::unique_ptr<NodePoolBase>(node.get()->copy()));
  }
}

NodeGraph &NodeGraph::operator=(const NodeGraph &other) {
  if (this != &other) {
    nodes = other.nodes;
    nodeData.clear();
    for (auto &node : other.nodeData) {
      nodeData.push_back(std::unique_ptr<NodePoolBase>(node.get()->copy()));
    }
  }
  return *this;
}

NodeGraph::NodeGraph() {
  nodeData.push_back(std::make_unique<NodePool<nForwarder>>());
  nodeData.push_back(std::make_unique<NodePool<nNumber>>());
  nodeData.push_back(std::make_unique<NodePool<nString>>());
  nodeData.push_back(std::make_unique<NodePool<nAdd>>());
  nodeData.push_back(std::make_unique<NodePool<nSub>>());
  nodeData.push_back(std::make_unique<NodePool<nMul>>());
  nodeData.push_back(std::make_unique<NodePool<nDiv>>());
  nodeData.push_back(std::make_unique<NodePool<nReturn>>());

  // Pool<nForwarder> *pool =
  //     &nodeData[0].get()->convertTo<NodePool<nForwarder>>()->pool;
  //
  // tHandle n1 = pool->add(nForwarder{.inputs = {{.pinIdx = 0, .nodeIdx =
  // 1}}}); tHandle n2 = pool->add(nForwarder{.inputs = {{.pinIdx = 0, .nodeIdx
  // = 0}}});
  //
  // nodes.add(Node{
  //     .selected = true,
  //     .node_data = n1,
  //     .templateIdx = N_FORWARDER,
  //     .position = {-100, 0},
  // });
  //
  // nodes.add(Node{
  //     .selected = false,
  //     .node_data = n2,
  //     .templateIdx = N_FORWARDER,
  //     .position = {100, 0},
  // });

  add_node(N_FORWARDER, nForwarder{.inputs = {0, 1}})->position[0] = -150;
  add_node(N_FORWARDER, nForwarder{.inputs = {-1, -1}})->position[0] = 150;

  add_node(N_NUMBER, nNumber{.value = 1})->setPos(0, 60);
  add_node(N_NUMBER, nNumber{.value = 2})->setPos(0, 120);
  add_node(N_ADD, nAdd{.value = 3})->setPos(120, 90);
  add_node(N_DIV, nDiv{.value = 4})->setPos(-300, 60);
  // add_node(N_STRING, nNumber{.value = 4})->setPos(-200, 120);

  add_node(N_RETURN, nReturn{})->setPos(200, 120);
}
