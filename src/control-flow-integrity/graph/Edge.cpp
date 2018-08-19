#include "control-flow-integrity/graph/Edge.h"

namespace cfi {
namespace graph {

Edge::Edge(Vertex org, Vertex dest) {
  origin = std::move(org);
  destination = std::move(dest);
}

Vertex Edge::getOrigin() const { return origin; }

Vertex Edge::getDestination() const { return destination; }

bool Edge::operator==(const Edge &rhs) const {
  return (origin == rhs.origin) && (destination == rhs.destination);
}

std::string Edge::str() const {
  std::ostringstream os;
  os << this->origin.getMethodName() << " " << this->destination.getMethodName();
  return os.str();
}

bool Edge::operator<(const Edge &other) const {
  return this->str() < other.str();
}
}
}