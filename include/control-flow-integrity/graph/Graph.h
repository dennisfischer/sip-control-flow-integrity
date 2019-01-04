#pragma once

#ifndef CFI_GRAPH_GRAPH_H
#define CFI_GRAPH_GRAPH_H

#include <unordered_set>
#include <vector>
#include "control-flow-integrity/graph/Vertex.h"
#include "control-flow-integrity/graph/Edge.h"

namespace cfi {
namespace graph {
class Graph {
private:
  std::unordered_set<Vertex> vertices;
  std::unordered_set<Edge> edges;
public:
  Graph();

  void addEdge(Vertex origin, Vertex destination);

  std::string str();

  std::vector<Vertex> getVertices() const;

  std::vector<Edge> getEdges() const;

  void removeVertex(Vertex origin);
};
}
}

#endif // CFI_GRAPH_GRAPH_H
