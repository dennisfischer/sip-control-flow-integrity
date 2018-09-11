#ifndef CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H
#define CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H

#include <composition/Analysis.hpp>
#include <self-checksumming/FunctionFilter.h>
#include <control-flow-integrity/graph/Graph.h>

namespace cfi {
#ifndef CONTROL_FLOW_INTEGRITY
#define CONTROL_FLOW_INTEGRITY "control_flow_integrity"
#endif

class ControlFlowIntegrityPass
    : public llvm::ModulePass, public composition::ComposableAnalysis<ControlFlowIntegrityPass> {
public:
  static char ID;
private:
  static graph::Graph graph;

  std::string cfi_guard_str = "sc_guard";
  llvm::MDNode *cfi_guard_md{};
public:
  ControlFlowIntegrityPass() : ModulePass(ID) {}

  bool runOnModule(llvm::Module &M) override;

  void getAnalysisUsage(llvm::AnalysisUsage &usage) const override;

  std::pair<std::set<llvm::Value *>, std::set<llvm::Instruction *>>
  applyCFI(llvm::Function &F, std::unordered_map<llvm::Function *, bool> funcAddressTaken);

  bool doFinalization(llvm::Module &module) override;
private:
  bool skip_function(llvm::Function &F, FunctionInformation *info);
};
}

#endif //CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H
