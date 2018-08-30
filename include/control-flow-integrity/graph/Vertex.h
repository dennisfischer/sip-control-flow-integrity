#pragma once

#ifndef CFI_GRAPH_VERTEX_H
#define CFI_GRAPH_VERTEX_H

#include <string>

namespace cfi {
namespace graph {

class Vertex {
private:
  std::string methodName;
  bool sensitive;
public:
  explicit Vertex(const std::string &method);

  Vertex(const std::string &method, bool sensitive);

  Vertex();

  std::string getMethodName() const;

  bool isSensitive();

  bool operator==(const Vertex &rhs) const;

  bool operator!=(const Vertex &rhs) const;

};
}
}

namespace std {
template<>
struct hash<cfi::graph::Vertex> {
  size_t operator()(const cfi::graph::Vertex &x) const {
    return hash<std::string>()(x.getMethodName());
  }
};
}
#endif //CFI_GRAPH_VERTEX_H
