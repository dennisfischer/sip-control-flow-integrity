#include <algorithm>
#include <sstream>
#include "control-flow-integrity/graph/Graph.h"

namespace cfi {
namespace graph {

Graph::Graph() = default;

void Graph::addEdge(Vertex origin, Vertex destination) {
  Edge newEdge(origin, destination);
  if (vertices.find(origin) == vertices.end()) {
    vertices.insert(origin);
  } else if (origin.isSensitive()) {
    vertices.erase(origin);
    vertices.insert(origin);
  }

  if (vertices.find(destination) == vertices.end()) {
    vertices.insert(destination);
  } else if (destination.isSensitive()) {
    vertices.erase(destination);
    vertices.insert(destination);
  }

  if (edges.find(newEdge) == edges.end()) {
    edges.insert(newEdge);
  }
}

std::vector<Vertex> Graph::getVertices() const {
  return std::vector<Vertex>(vertices.begin(), vertices.end());
}

std::vector<Edge> Graph::getEdges() const {
  return std::vector<Edge>(edges.begin(), edges.end());
}

std::string Graph::str() {
  std::ostringstream os;
  for (const Edge &e : this->edges) {
    os << e.str() << std::endl;
  }
  return os.str();
}
}
}