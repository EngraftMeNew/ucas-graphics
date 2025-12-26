//
// Created by creeper on 7/20/24.
//
#include <meshark/mesh-simplifier.h>
#include <format>

namespace meshark {

Vertex MeshSimplifier::collapseEdge(Edge e) {
  // TODO: Implement this function
  return Vertex();
}

MeshSimplifier::MinCostEdgeCollapsingResult MeshSimplifier::collapseMinCostEdge() {
  auto min_cost_edge = cost_edge_map.begin()->second;
  // TODO: finish this function
  return {Edge(), false};
}

Real MeshSimplifier::computeEdgeCost(Edge e) const {
  // TODO: Implement this function
 return 0.0;
}

void MeshSimplifier::runSimplify(Real alpha) {
  for (auto v : mesh.vertices())
    Q(v) = computeQuadricMatrix(v);
  for (auto e : mesh.edges()) {
    edge_collapse_cost(e) = computeEdgeCost(e);
    cost_edge_map.insert({edge_collapse_cost(e), e});
  }
  int round = 0;
  while (mesh.numEdges() > alpha * num_original_edges) {
    auto result = collapseMinCostEdge();
    round++;
    std::cout << std::format("Round {}: ", round);
    if (!result.is_collapsable) {
      auto e = result.failed_edge;
      updateEdgeCost(e, std::numeric_limits<Real>::infinity());
      std::cout << "Min-cost edge is not collapsable, skip\n";
      continue;
    }
    std::cout << std::format("{} edges left\n", mesh.numEdges());
  }
}

glm::vec3 MeshSimplifier::computeOptimalCollapsePosition(Edge e) const {
  // TODO: implement this function
  return glm::vec3(0.f);
}

void MeshSimplifier::updateVertexPos(Vertex v, const glm::vec3 &pos) {
  // TODO: implement this function

}

glm::mat4 MeshSimplifier::computeQuadricMatrix(Vertex v) const {
 // TODO: implement this function

  return glm::mat4(1.0f);
}

void MeshSimplifier::eraseEdgeMapping(Edge e) {
  Real cost = edge_collapse_cost(e);
  auto range = cost_edge_map.equal_range(cost);
  assert(range.first != cost_edge_map.end());
  for (auto it = range.first; it != range.second; ++it) {
    if (it->second == e) {
      cost_edge_map.erase(it);
      break;
    }
  }
}
}