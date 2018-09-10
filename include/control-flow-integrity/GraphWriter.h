#ifndef CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H
#define CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H

#include <llvm/Pass.h>
#include <control-flow-integrity/graph/Graph.h>

namespace cfi {
class GraphWriter  {
private:
  graph::Graph graph;
  std::string classTemplate;
public:
  explicit GraphWriter(const graph::Graph &graph, const std::string &classTemplate);

  void write();

  void writeStatsFile(const std::string &filename, const std::vector<graph::Vertex> &registeredVertices);
private:
  void rewriteStackAnalysis(const std::string &edges, size_t numVertices, size_t numEdges);

  std::vector<graph::Vertex> getPathsToSensitiveNodes();

  std::vector<graph::Vertex> getCallees(const graph::Vertex &v);

  std::vector<graph::Vertex> getCallers(const graph::Vertex &v);

  std::vector<graph::Vertex> getSensitiveNodes();
};
}

#endif //CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H
