#ifndef CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H
#define CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H

#include <llvm/Pass.h>
#include <control-flow-integrity/graph/Graph.h>

namespace cfi {
class GraphWriter {
private:
  graph::Graph graph;
public:
  explicit GraphWriter(const graph::Graph &graph);

  void write(const std::string &outPath, const std::string &classTemplate);
private:
  void rewriteStackAnalysis(const std::string &checksum, const std::string &outPath, const std::string &classTemplate);

  std::vector<graph::Vertex> getPathsToSensitiveNodes();

  std::vector<graph::Vertex> getCallees(const graph::Vertex &v);

  std::vector<graph::Vertex> getCallers(const graph::Vertex &v);

  std::vector<graph::Vertex> getSensitiveNodes();

  std::string hashFile(FILE *inFile) const;
};
}

#endif //CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H
