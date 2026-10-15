#include "Any_and_Pool.hpp"
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
  nodeData.push_back(std::make_unique<NodePool<nVariable>>());
  nodeData.push_back(std::make_unique<NodePool<nAdd>>());
  nodeData.push_back(std::make_unique<NodePool<nSub>>());
  nodeData.push_back(std::make_unique<NodePool<nMul>>());
  nodeData.push_back(std::make_unique<NodePool<nDiv>>());
  nodeData.push_back(std::make_unique<NodePool<nSet>>());
  nodeData.push_back(std::make_unique<NodePool<nEqual>>());
  nodeData.push_back(std::make_unique<NodePool<nLess>>());
  nodeData.push_back(std::make_unique<NodePool<nMore>>());
  nodeData.push_back(std::make_unique<NodePool<nLessOrEqual>>());
  nodeData.push_back(std::make_unique<NodePool<nMoreOrEqual>>());
  nodeData.push_back(std::make_unique<NodePool<nNot>>());
  nodeData.push_back(std::make_unique<NodePool<nIf>>());
  nodeData.push_back(std::make_unique<NodePool<nLoop>>());
  nodeData.push_back(std::make_unique<NodePool<nReturn>>());
  nodeData.push_back(std::make_unique<NodePool<nStart>>());

  add_node(N_START)->setPos(0, 0);
}

bool NodeGraph::calculate() {
  // for (auto &node : nodes)
  //   node.computed = false;

  // nReturn *rn = nodeData[(size_t)N_RETURN]
  //                   .get()
  //                   ->convertTo<NodePool<nReturn>>()
  //                   ->pool.get(tHandle{.gen = 1, .idx = 0});
  //
  // if (rn->inputs[0] != Link{-1, -1})
  //   rn->value = recursiveDescend(rn->inputs[0]);

  return false;
}

Any NodeGraph::recursiveDescend(Link olink) {
  // Node *node = nodes.get(tHandle{.gen = 0, .idx = olink.nodeIdx}, true);
  // void *data = nodeData[node->templateIdx].get()->get(node->node_data);
  //
  // NodeGraph_NodeType nt = (NodeGraph_NodeType)node->templateIdx;
  // if (nt == N_NUMBER) {
  //   return Any{((nNumber *)data)->value};
  // } else {
  //   return Any{0.0f};
  // }
}
