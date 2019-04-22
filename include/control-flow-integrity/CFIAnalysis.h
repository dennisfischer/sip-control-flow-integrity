#ifndef CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H
#define CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H

#include <composition/support/Analysis.hpp>
#include <function-filter/Filter.hpp>
#include <control-flow-integrity/graph/Graph.h>

namespace cfi {
#ifndef CONTROL_FLOW_INTEGRITY
#define CONTROL_FLOW_INTEGRITY "control_flow_integrity"
#endif

class ControlFlowIntegrityPass : public composition::support::ComposableAnalysis<ControlFlowIntegrityPass> {
public:
  static char ID;
private:
  std::shared_ptr<graph::Graph> graph;

  std::string cfi_guard_str = "cfi_guard";
  llvm::MDNode *cfi_guard_md{};
public:
  ControlFlowIntegrityPass() = default;

  bool runOnModule(llvm::Module &M) override;

  void getAnalysisUsage(llvm::AnalysisUsage &usage) const override;

  std::set<llvm::Value *> applyCFI(llvm::Function &F, const std::unordered_map<llvm::Function *, bool>& funcAddressTaken);

  bool doFinalization(llvm::Module &module) override;
private:
  bool skip_function(llvm::Function &F, FunctionInformation *info);
};
}

#endif //CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H
