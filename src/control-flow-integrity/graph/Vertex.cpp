#include "control-flow-integrity/graph/Vertex.h"

namespace cfi {
namespace graph {
Vertex::Vertex(const std::string &method) {
  methodName = method;
  sensitive = false;
}

Vertex::Vertex(const std::string &method, bool sensitive) {
  methodName = method;
  this->sensitive = sensitive;
}

Vertex::Vertex() {
  methodName = "";
  sensitive = false;
}

std::string Vertex::getMethodName() const { return methodName; }

bool Vertex::isSensitive() { return sensitive; }

bool Vertex::operator==(const Vertex &rhs) const {
  return (methodName == rhs.methodName);
}

bool Vertex::operator!=(const Vertex &rhs) const {
  return (methodName != rhs.methodName);
}
}
}