#include <llvm/IR/TypeBuilder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <control-flow-integrity/CFIAnalysis.h>
#include <control-flow-integrity/GraphWriter.h>

using namespace llvm;
using namespace composition;

namespace cfi {
static RegisterPass<ControlFlowIntegrityPass>
    X("control-flow-integrity", "Control Flow Integrity Pass", false, false);

cl::opt<std::string> StackAnalysisTemplate
    ("cfi-template", cl::Hidden, cl::desc("File path to the source file template used for the StackAnalysis"));

char ControlFlowIntegrityPass::ID = 0;
graph::Graph ControlFlowIntegrityPass::graph = {};

bool ControlFlowIntegrityPass::runOnModule(Module &M) {
  for (auto &F : M) {
    auto[undoValues, guardValues] = this->applyCFI(F);
    auto m = std::shared_ptr<Manifest>(new Manifest(
        "cfi",
        &F,
        [](const Manifest &) {},
        {},
        false,
        undoValues,
        guardValues
    ));
    addProtection(m);
  }
  return true;
}

bool ControlFlowIntegrityPass::doFinalization(Module &module) {
  dbgs() << "Finalizing...\n";
  GraphWriter g{graph, StackAnalysisTemplate.getValue()};
  g.write();

  return ModulePass::doFinalization(module);
}

std::pair<std::set<llvm::Value *>, std::set<llvm::Instruction *>> ControlFlowIntegrityPass::applyCFI(Function &F) {
  std::set<llvm::Value *> undoValues{};
  std::set<llvm::Instruction *> guardValues{};
  std::string funcName = F.getName().str();
  dbgs() << "CFI Running on: " << funcName << ".\n";
  bool first_instr = true;
  for (auto &BB : F) {
    for (auto &I: BB) {
      if (first_instr) {
        LLVMContext &Ctx = F.getContext();

        FunctionType *registerType = TypeBuilder<void(char *), false>::get(Ctx);
        auto registerFunction = cast<Function>(F.getParent()->getOrInsertFunction("registerFunction", registerType));

        IRBuilder<> builder(&I);
        builder.SetInsertPoint(&BB, builder.GetInsertPoint());

        Value *strPtr = builder.CreateGlobalStringPtr(funcName);
        undoValues.insert(builder.CreateCall(registerFunction, strPtr));
        first_instr = false;

        // Function is in the sensitive list
        if (F.hasFnAttribute(CONTROL_FLOW_INTEGRITY)) {
          FunctionType *verifyType = TypeBuilder<void(), false>::get(Ctx);
          auto verifyFunction = cast<Function>(F.getParent()->getOrInsertFunction("verifyStack", verifyType));

          // Insert call
          builder.SetInsertPoint(&BB, builder.GetInsertPoint());
          auto call = builder.CreateCall(verifyFunction);
          undoValues.insert(call);
          guardValues.insert(call);
        }
      }
      if (auto *callInstruction = dyn_cast<CallInst>(&I)) {
        Function *called = callInstruction->getCalledFunction();
        if (called) {
          std::string calledName = called->getName().str();
          graph::Vertex calledVertex;
          if (called->hasFnAttribute(CONTROL_FLOW_INTEGRITY)) {
            calledVertex = graph::Vertex(calledName, true);
          } else {
            calledVertex = graph::Vertex(calledName);
          }
          auto funcVertex = graph::Vertex(funcName);
          graph.addEdge(funcVertex, calledVertex);
        }
      }
      if (isa<ReturnInst>(&I)) {
        LLVMContext &Ctx = F.getContext();

        FunctionType *registerType = TypeBuilder<void(char *), false>::get(Ctx);
        auto
            deregisterFunction = cast<Function>(F.getParent()->getOrInsertFunction("deregisterFunction", registerType));

        IRBuilder<> builder(&I);
        builder.SetInsertPoint(&BB, builder.GetInsertPoint());

        // Insert a call to our function.
        Value *strPtr = builder.CreateGlobalStringPtr(funcName);
        undoValues.insert(builder.CreateCall(deregisterFunction, strPtr));
      }
    }
  }
  return {undoValues, guardValues};
}

void ControlFlowIntegrityPass::getAnalysisUsage(AnalysisUsage &usage) const {
  usage.setPreservesAll();
}

graph::Graph &ControlFlowIntegrityPass::getGraph() {
  return graph;
}
}