//
// Created by creeper on 7/20/24.
//
#include <meshark/mesh-simplifier.h>
#include <format>

namespace meshark
{

  Vertex MeshSimplifier::collapseEdge(Edge e)
  {
    // TODO: [TASK2] Implement this function
    // 6个半边，3条边，2个面，1个点

    // ============================
    // 会用到/会被修改的元素
    // ============================
    HalfEdge h01 = e->halfEdge();
    HalfEdge h10 = h01->twin;

    // 左右两侧面
    Face f0 = h01->face;
    Face f1 = h10->face;

    // 三角形的另外两个半边
    HalfEdge h12 = h01->next; // v1 -> v2
    HalfEdge h20 = h12->next; // v2 -> v0

    HalfEdge h03 = h10->next; // v0 -> v3
    HalfEdge h31 = h03->next; // v3 -> v1

    Vertex v0 = h01->tail; // keep
    Vertex v1 = h01->tip;  // remove
    Vertex v2 = h12->tip;  // 共同邻居
    Vertex v3 = h03->tip;  // 共同邻居

    Edge e01 = e;         // v0-v1, 需要删除
    Edge e12 = h12->edge; // v1-v2，需要删除
    Edge e20 = h20->edge; // v2-v0 (keep)
    Edge e03 = h03->edge; // v0-v3 (keep)
    Edge e31 = h31->edge; // v3-v1，需要删除

    // 需要缝合的两条边的对边半边
    // 边 (v0, v2)：保留 h02，并让 h21 成为它的对边（h21 原本指向 v1，之后会改为指向 v0）
    HalfEdge h02 = h20->twin; // v0 -> v2
    HalfEdge h21 = h12->twin; // v2 -> v1

    HalfEdge h30 = h03->twin; // v3 -> v0
    HalfEdge h13 = h31->twin; // v1 -> v3

    // ============================
    // 修改指针
    // ============================

    // 把所有原本从 v1 出发的半边，改成从 v0 出发
    std::vector<HalfEdge> outgoing_v1;
    outgoing_v1.reserve(v1->degree());
    for (HalfEdge h : v1->outgoingHalfEdges())
      outgoing_v1.push_back(h);

    for (HalfEdge h : outgoing_v1)
    {
      if (!h)
        continue;
      if (h == h10 || h == h12)
        continue;
      h->tail = v0;
      h->twin->tip = v0;
    }

    // h21的tip改为v0
    h21->tip = v0;

    // 在 v2 周围合并重复边：把 (v1-v2) 合并进 (v0-v2)
    // 让 h02 与 h21 互为对边，并且它们都属于 e20
    h02->twin = h21;
    h21->twin = h02;
    h02->edge = e20;
    h21->edge = e20;
    e20->halfEdge() = h02;

    //  在 v3 周围合并重复边：把 (v1-v3) 合并进 (v0-v3)
    // h13 原本是 v1->v3，合并后变为 v0->v3，并与 h30 互为对边，且都属于 e03
    h13->tail = v0;
    h13->twin = h30;
    h30->twin = h13;
    h13->edge = e03;
    h30->edge = e03;
    e03->halfEdge() = h13;

    // 修改顶点的halfedge指针
    v0->halfEdge() = h02;
    if (v2->halfEdge() == h20)
      v2->halfEdge() = h21;
    if (v3->halfEdge() == h31)
      v3->halfEdge() = h30;

    // ============================
    // 删除不需要的元素
    // ============================

    // 删除边之前，先把 MeshSimplifier 维护的边相关数据从map/EdgeData里移除
    eraseEdgeMapping(e01);
    eraseEdgeMapping(e12);
    eraseEdgeMapping(e31);
    edge_collapse_cost.removeEdgeData(e01);
    edge_collapse_cost.removeEdgeData(e12);
    edge_collapse_cost.removeEdgeData(e31);

    // 删除顶点之前，先移除该顶点的Quadric数据
    Q.removeVertexData(v1);

    // 清空半边指针
    auto detach_halfedge = [](HalfEdge h)
    {
      if (!h)
        return;
      h->next = static_cast<HalfEdge>(nullptr);
      h->twin = static_cast<HalfEdge>(nullptr);
      h->face = static_cast<Face>(nullptr);
      h->edge = static_cast<Edge>(nullptr);
      h->tip = static_cast<Vertex>(nullptr);
      h->tail = static_cast<Vertex>(nullptr);
    };

    // 先删除两个面
    mesh.removeFace(f0);
    mesh.removeFace(f1);

    // 再删除这两个面上的 6 条半边
    detach_halfedge(h01);
    detach_halfedge(h10);
    detach_halfedge(h12);
    detach_halfedge(h20);
    detach_halfedge(h03);
    detach_halfedge(h31);

    mesh.removeHalfEdge(h01);
    mesh.removeHalfEdge(h10);
    mesh.removeHalfEdge(h12);
    mesh.removeHalfEdge(h20);
    mesh.removeHalfEdge(h03);
    mesh.removeHalfEdge(h31);

    // 删除消失的 3 条边： e01、 e12、e31
    mesh.removeEdge(e01);
    mesh.removeEdge(e12);
    mesh.removeEdge(e31);

    // 删除 v1
    mesh.removeVertex(v1);

    return v0;
  }

  MeshSimplifier::MinCostEdgeCollapsingResult MeshSimplifier::collapseMinCostEdge()
  {
    auto min_cost_edge = cost_edge_map.begin()->second;
    // TODO: finish this function
    return {Edge(), false};
  }

  Real MeshSimplifier::computeEdgeCost(Edge e) const
  {
    // TODO: Implement this function
    return 0.0;
  }

  void MeshSimplifier::runSimplify(Real alpha)
  {
    for (auto v : mesh.vertices())
      Q(v) = computeQuadricMatrix(v);
    for (auto e : mesh.edges())
    {
      edge_collapse_cost(e) = computeEdgeCost(e);
      cost_edge_map.insert({edge_collapse_cost(e), e});
    }
    int round = 0;
    while (mesh.numEdges() > alpha * num_original_edges)
    {
      auto result = collapseMinCostEdge();
      round++;
      std::cout << std::format("Round {}: ", round);
      if (!result.is_collapsable)
      {
        auto e = result.failed_edge;
        updateEdgeCost(e, std::numeric_limits<Real>::infinity());
        std::cout << "Min-cost edge is not collapsable, skip\n";
        continue;
      }
      std::cout << std::format("{} edges left\n", mesh.numEdges());
    }
  }

  glm::vec3 MeshSimplifier::computeOptimalCollapsePosition(Edge e) const
  {
    // TODO: implement this function
    return glm::vec3(0.f);
  }

  void MeshSimplifier::updateVertexPos(Vertex v, const glm::vec3 &pos)
  {
    // TODO: implement this function
  }

  glm::mat4 MeshSimplifier::computeQuadricMatrix(Vertex v) const
  {
    // TODO: implement this function

    return glm::mat4(1.0f);
  }

  void MeshSimplifier::eraseEdgeMapping(Edge e)
  {
    Real cost = edge_collapse_cost(e);
    auto range = cost_edge_map.equal_range(cost);
    assert(range.first != cost_edge_map.end());
    for (auto it = range.first; it != range.second; ++it)
    {
      if (it->second == e)
      {
        cost_edge_map.erase(it);
        break;
      }
    }
  }
}