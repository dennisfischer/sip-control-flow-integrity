#pragma once

#ifndef CFI_GRAPH_GRAPH_H
#define CFI_GRAPH_GRAPH_H

#include <vector>
#include <algorithm>
#include "control-flow-integrity/graph/Vertex.h"
#include "control-flow-integrity/graph/Edge.h"

namespace cfi {
namespace graph {
class Graph {
private:
  std::vector<Vertex> vertices;
  std::vector<Edge> edges;
public:
  Graph();

  bool contains(Vertex v);

  bool contains(Edge e);

  void insert(Vertex v);

  void insert(Edge e);

  void addEdge(Vertex origin, Vertex destination);

  std::string str();

  std::vector<Vertex> getVertices() const;

  std::vector<Edge> getEdges() const;
};
}
}

#endif // CFI_GRAPH_GRAPH_H
