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
  nodeData.push_back(std::make_unique<NodePool<nBool>>());
  nodeData.push_back(std::make_unique<NodePool<nNumber>>());
  nodeData.push_back(std::make_unique<NodePool<nString>>());
  nodeData.push_back(std::make_unique<NodePool<nAdd>>());
  nodeData.push_back(std::make_unique<NodePool<nSub>>());
  nodeData.push_back(std::make_unique<NodePool<nMul>>());
  nodeData.push_back(std::make_unique<NodePool<nDiv>>());
  nodeData.push_back(std::make_unique<NodePool<nEqual>>());
  nodeData.push_back(std::make_unique<NodePool<nLess>>());
  nodeData.push_back(std::make_unique<NodePool<nMore>>());
  nodeData.push_back(std::make_unique<NodePool<nLessOrEqual>>());
  nodeData.push_back(std::make_unique<NodePool<nMoreOrEqual>>());
  nodeData.push_back(std::make_unique<NodePool<nNot>>());
  nodeData.push_back(std::make_unique<NodePool<nIf>>());
  nodeData.push_back(std::make_unique<NodePool<nLoop>>());
  nodeData.push_back(std::make_unique<NodePool<nReturn>>());

  add_node(N_RETURN, nReturn{})->setPos(200, 120);
}
