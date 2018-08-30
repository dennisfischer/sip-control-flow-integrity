#ifndef CFI_GRAPH_EDGE_H
#define CFI_GRAPH_EDGE_H

#include "control-flow-integrity/graph/Vertex.h"

namespace cfi {
namespace graph {

class Edge {
private:
  Vertex origin;
  Vertex destination;
public:
  Edge(Vertex org, Vertex dest);

  Vertex getOrigin() const;

  Vertex getDestination() const;

  bool operator==(const Edge &rhs) const;;

  std::string str() const;

  bool operator<(const Edge &other) const;
};
}
}
namespace std {
template<>
struct hash<cfi::graph::Edge> {
  size_t operator()(const cfi::graph::Edge &x) const {
    return (hash<cfi::graph::Vertex>()(x.getOrigin()) << 1) ^ hash<cfi::graph::Vertex>()(x.getDestination());
  }
};
}
#endif //CFI_GRAPH_EDGE_H
