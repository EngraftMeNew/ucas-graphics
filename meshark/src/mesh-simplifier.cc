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

    // 删除顶点之前：先移除该顶点的 Quadric 数据
    Q.removeVertexData(v1);

    // 先删除两个面（此时 face->halfEdge 等引用仍然有效）
    mesh.removeFace(f0);
    mesh.removeFace(f1);

    // 逐条删除将消失的 3 条边，并同步移除 map / EdgeData，避免“交换删除”导致索引不同步
    auto remove_edge_with_data = [&](Edge ed)
    {
      if (!ed)
        return;
      eraseEdgeMapping(ed);
      edge_collapse_cost.removeEdgeData(ed);
      mesh.removeEdge(ed);
    };
    remove_edge_with_data(e01);
    remove_edge_with_data(e12);
    remove_edge_with_data(e31);

    // 再删除两个面上的 6 条半边
    mesh.removeHalfEdge(h01);
    mesh.removeHalfEdge(h10);
    mesh.removeHalfEdge(h12);
    mesh.removeHalfEdge(h20);
    mesh.removeHalfEdge(h03);
    mesh.removeHalfEdge(h31);

    // 最后删除被合并掉的顶点 v1
    mesh.removeVertex(v1);

    return v0;
  }

  MeshSimplifier::MinCostEdgeCollapsingResult MeshSimplifier::collapseMinCostEdge()
  {
    auto min_cost_edge = cost_edge_map.begin()->second;
    // TODO: [TASK4] finish this function
    // 如果最小代价边不可坍缩：返回该边，并将其从 cost_edge_map 中移除，避免反复选中同一条边
    if (!mesh.isCollapsable(min_cost_edge))
    {
      eraseEdgeMapping(min_cost_edge);
      return {min_cost_edge, false};
    }

    // 最优坍缩位置
    glm::vec3 opt_pos = computeOptimalCollapsePosition(min_cost_edge);

    // 坍缩
    Vertex v = collapseEdge(min_cost_edge);

    //  更新坍缩后顶点位置
    updateVertexPos(v, opt_pos);

    return {Edge(), true};
  }

  Real MeshSimplifier::computeEdgeCost(Edge e) const
  {
    // TODO: [TASK3] Implement this function
    HalfEdge h = e->halfEdge();
    Vertex v0 = h->tail;
    Vertex v1 = h->tip;

    glm::mat4 Qbar = Q(v0) + Q(v1);

    glm::vec3 p = computeOptimalCollapsePosition(e);
    glm::vec4 hp(p, 1.0f);

    // 代价：p^T Q p
    glm::vec4 Qhp = Qbar * hp;
    Real cost = static_cast<Real>(glm::dot(hp, Qhp));
    return cost;
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
    // TODO: [TASK3] implement this function
    HalfEdge h = e->halfEdge();
    Vertex v0 = h->tail;
    Vertex v1 = h->tip;

    glm::mat4 Qbar = Q(v0) + Q(v1);

    // 解线性系统：A * x = -b
    // A 为 Qbar 左上角 3x3，b 为 Qbar 的第 4 列前三维 (q03, q13, q23)
    glm::mat3 A = glm::mat3(Qbar);
    glm::vec3 b = glm::vec3(Qbar[3]);

    auto eval_cost = [&](const glm::vec3 &p) -> Real
    {
      glm::vec4 hp(p, 1.0f);
      glm::vec4 Qhp = Qbar * hp;
      return static_cast<Real>(glm::dot(hp, Qhp));
    };

    const Real det = static_cast<Real>(glm::determinant(A));
    const Real eps = static_cast<Real>(1e-12);

    if (std::abs(det) > eps)
    {
      glm::vec3 x = glm::inverse(A) * (-b);
      return x;
    }

    // A 不可逆：退化情况，尝试端点与中点，取代价最小者
    glm::vec3 p0 = mesh.pos(v0);
    glm::vec3 p1 = mesh.pos(v1);
    glm::vec3 pm = (p0 + p1) * 0.5f;

    glm::vec3 best_p = p0;
    Real best_cost = eval_cost(p0);

    Real c1 = eval_cost(p1);
    if (c1 < best_cost)
    {
      best_cost = c1;
      best_p = p1;
    }

    Real cm = eval_cost(pm);
    if (cm < best_cost)
    {
      best_cost = cm;
      best_p = pm;
    }

    return best_p;
  }

  void MeshSimplifier::updateVertexPos(Vertex v, const glm::vec3 &pos)
  {
    // TODO: [TASK3] implement this function
    // 更新顶点位置
    mesh.setVertexPos(v, pos);

    // 收集需要更新 Q 的顶点：所有与 v 相邻的面片上的顶点
    std::vector<Vertex> affected_vertices;
    affected_vertices.reserve(v->degree() + 4);

    auto push_unique_vertex = [&](Vertex x)
    {
      if (!x)
        return;
      for (Vertex y : affected_vertices)
        if (y == x)
          return;
      affected_vertices.push_back(x);
    };

    push_unique_vertex(v);

    for (HalfEdge h : v->outgoingHalfEdges())
    {
      if (!h)
        continue;
      push_unique_vertex(h->tip);

      Face f = h->face;
      if (!f)
        continue;
      for (HalfEdge fh : f->boundaryHalfEdges())
      {
        push_unique_vertex(fh->tip);
      }
    }

    // 更新这些顶点的 Q
    for (Vertex u : affected_vertices)
    {
      Q(u) = computeQuadricMatrix(u);
    }

    // 收集需要更新代价的边：所有与这些顶点相邻的边
    std::vector<Edge> affected_edges;
    affected_edges.reserve(affected_vertices.size() * 6);

    auto push_unique_edge = [&](Edge x)
    {
      if (!x)
        return;
      for (Edge y : affected_edges)
        if (y == x)
          return;
      affected_edges.push_back(x);
    };

    for (Vertex u : affected_vertices)
    {
      for (HalfEdge h : u->outgoingHalfEdges())
      {
        if (!h)
          continue;
        push_unique_edge(h->edge);
      }
    }

    // 重新计算这些边的代价，并更新 multimap
    for (Edge e : affected_edges)
    {
      if (!e)
        continue;
      Real new_cost = computeEdgeCost(e);
      updateEdgeCost(e, new_cost);
    }
  }

  glm::mat4 MeshSimplifier::computeQuadricMatrix(Vertex v) const
  {
    // TODO: [TASK3] implement this function
    glm::mat4 Qv(0.0f);
    glm::vec3 pv = mesh.pos(v);

    // Q(v) = Σ_f (p p^T), 其中平面 p=(a,b,c,d), ax+by+cz+d=0
    for (HalfEdge h : v->outgoingHalfEdges())
    {
      if (!h)
        continue;
      Face f = h->face;
      if (!f)
        continue;

      glm::vec3 n = mesh.normal(f);
      n = glm::normalize(n);

      Real d = static_cast<Real>(-glm::dot(n, pv));
      glm::vec4 plane(n, static_cast<float>(d));

      Qv += glm::outerProduct(plane, plane);
    }

    return Qv;
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
