#ifndef CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H
#define CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H

#include <cstdio>
#include <fstream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <openssl/sha.h>
#include <regex>
#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/TypeBuilder.h>
#include "graph/Graph.h"
#include <composition/Analysis.hpp>

#ifndef CONTROL_FLOW_INTEGRITY
#define CONTROL_FLOW_INTEGRITY "control_flow_integrity"
#endif

namespace cfi {
class ControlFlowIntegrityPass
    : public llvm::ModulePass, public composition::ComposableAnalysis<ControlFlowIntegrityPass> {
public:
  static char ID;
private:
  static graph::Graph graph;
public:
  ControlFlowIntegrityPass() : ModulePass(ID) {}

  bool runOnModule(llvm::Module &M) override;

  void getAnalysisUsage(llvm::AnalysisUsage &usage) const override;

  graph::Graph &getGraph() {
    return graph;
  }

  std::set<llvm::Value *> applyCFI(llvm::Function &F);
};
}

#endif //CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H
