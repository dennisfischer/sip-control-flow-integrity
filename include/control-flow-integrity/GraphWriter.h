#ifndef CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H
#define CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H

#include <composition/Protection.hpp>
#include <llvm/Pass.h>
#include <control-flow-integrity/graph/Graph.h>

namespace cfi {
class ControlFlowIntegrityGraphPass
    : public llvm::ModulePass, public composition::ComposableProtection<ControlFlowIntegrityGraphPass> {
public:
  static char ID;
private:
  graph::Graph graph;
public:
  ControlFlowIntegrityGraphPass() : ModulePass(ID) {}

  bool runOnModule(llvm::Module &M) override;

  void getAnalysisUsage(llvm::AnalysisUsage &usage) const override;

private:
  void writeGraphFile(const graph::Graph &graph);

  void rewriteStackAnalysis(const std::string &checksum);

  std::vector<graph::Vertex> getPathsToSensitiveNodes();

  std::vector<graph::Vertex> getCallees(const graph::Vertex &v);

  std::vector<graph::Vertex> getCallers(const graph::Vertex &v);

  std::vector<graph::Vertex> getSensitiveNodes();

  std::string hashFile(FILE *inFile) const;
};
}

#endif //CONTROL_FLOW_INTEGRITY_GRAPHWRITER_H
