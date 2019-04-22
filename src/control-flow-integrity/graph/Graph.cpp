#include <algorithm>
#include <sstream>
#include "control-flow-integrity/graph/Graph.h"

namespace cfi::graph {

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

void Graph::removeVertex(const Vertex& origin) {
  std::unordered_set<Vertex> inNodes{};
  std::unordered_set<Vertex> outNodes{};

  for(const Edge &e : this->edges) {
    if(e.getOrigin() == e.getDestination()) {
      continue;
    } else if(e.getOrigin() == origin) {
      outNodes.insert(e.getDestination());
    } else if (e.getDestination() == origin) {
      inNodes.insert(e.getOrigin());
    }
  }
  
  for(auto it = edges.begin(), it_end = edges.end(); it != it_end; ) {
    if(it->getOrigin() == origin || it->getDestination() == origin) {
      it = edges.erase(it);
    } else {
      ++it;
    }
  }
  
  for(const Vertex &in : inNodes) {
    for (const Vertex &out : outNodes) {
      edges.insert(Edge{in, out});
    }
  }
}
}