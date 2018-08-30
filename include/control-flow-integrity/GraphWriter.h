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
private:
  void rewriteStackAnalysis(const std::string &checksum);

  std::vector<graph::Vertex> getPathsToSensitiveNodes();

  std::vector<graph::Vertex> getCallees(const graph::Vertex &v);

  std::vector<graph::Vertex> getCallers(const graph::Vertex &v);

  std::vector<graph::Vertex> getSensitiveNodes();

  std::string hashFile(FILE *inFile) const;
};
}

#endif //CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H
