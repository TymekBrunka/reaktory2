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

  Pool<nForwarder> *pool =
      &nodeData[0].get()->convertTo<NodePool<nForwarder>>()->pool;

  tHandle n1 = pool->add(nForwarder{.inputs = {{.pinIdx = 0, .nodeIdx = 1}}});
  tHandle n2 = pool->add(nForwarder{.inputs = {{.pinIdx = 0, .nodeIdx = 0}}});

  nodes.push_back(Node{
      .selected = true,
      .node_data = n1,
      .templateIdx = 0,
      .position = {-100, 0},
  });

  nodes.push_back(Node{
      .selected = false,
      .node_data = n2,
      .templateIdx = 0,
      .position = {100, 0},
  });
}
