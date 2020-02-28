#include <utility>
#include <composition/Manifest.hpp>
#include <control-flow-integrity/CFIAnalysis.h>
#include <control-flow-integrity/GraphWriter.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/TypeBuilder.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace composition;

namespace cfi {
static RegisterPass<ControlFlowIntegrityPass>
    X("control-flow-integrity", "Control Flow Integrity Pass", false, false);

cl::opt<std::string> StackAnalysisTemplate(
    "cfi-template", cl::Hidden,
    cl::desc(
        "File path to the source file template used for the StackAnalysis"));

cl::opt<std::string> OutputDirectory(
    "cfi-outputdir", cl::init("."),
    cl::desc("File path to the graph file for the StackAnalysis"));

char ControlFlowIntegrityPass::ID = 0;

class UndoManifest : public Manifest {
private:
  graph::Vertex v;
  std::shared_ptr<graph::Graph> g;

public:
  UndoManifest(
      graph::Vertex v, std::shared_ptr<graph::Graph> g, std::string name,
      llvm::Value *protectee, llvm::Value *blockProtectee,
      PatchFunction patchFunction,
      std::vector<std::shared_ptr<composition::graph::constraint::Constraint>>
      constraints = {},
      bool postPatching = false, std::set<llvm::Value *> undoValues = {})
      : Manifest(std::move(name), protectee, blockProtectee, std::move(patchFunction), std::move(constraints),
                 postPatching, std::move(undoValues)),
        v(std::move(v)), g(std::move(g)) {}

  void Undo() override {
    Manifest::Undo();
    g->removeVertex(v);
  }
};

bool ControlFlowIntegrityPass::runOnModule(Module &M) {
  graph = std::make_shared<graph::Graph>();
  auto function_filter_info =
      getAnalysis<FunctionFilterPass>().get_functions_info();

  std::unordered_map<llvm::Function *, bool> funcAddressTaken{};

  for (auto &F : M) {
    if (F.hasAddressTaken()) {
      funcAddressTaken.insert({&F, false});
    }

    if (skip_function(F, function_filter_info)) {
      continue;
    }
    if (!F.hasFnAttribute(CONTROL_FLOW_INTEGRITY)) {
      F.addFnAttr(CONTROL_FLOW_INTEGRITY);
    }
  }

  auto *cfi_guard_md_str = llvm::MDString::get(M.getContext(), cfi_guard_str);
  cfi_guard_md = llvm::MDNode::get(M.getContext(), cfi_guard_md_str);

  for (auto &F : M) {
    auto undoValues = this->applyCFI(F, funcAddressTaken);
    if (F.hasFnAttribute(CONTROL_FLOW_INTEGRITY)) {
      auto m = new UndoManifest(graph::Vertex(F.getName().str()), graph, "cfi",
                                nullptr, &F, [](const Manifest &) {}, {}, false,
                                undoValues);
      addProtection(m);
    }

    F.setMetadata(cfi_guard_str, cfi_guard_md);
  }
  return true;
}

bool ControlFlowIntegrityPass::doFinalization(Module &module) {
  GraphWriter g{graph};
  g.write(OutputDirectory.getValue(), StackAnalysisTemplate.getValue());

  return ModulePass::doFinalization(module);
}
void setMetadata(LLVMContext &ctx, Instruction *inst, std::string metadataStr){
    auto* guard_md_str = llvm::MDString::get(ctx,metadataStr);
    MDNode* guard_md = llvm::MDNode::get(ctx,guard_md_str);
    inst->setMetadata(metadataStr, guard_md);
}
std::set<llvm::Value *> ControlFlowIntegrityPass::applyCFI(
    Function &F, const std::unordered_map<llvm::Function *, bool> &funcAddressTaken) {
  std::set<llvm::Value *> undoValues{};
  std::string funcName = F.getName().str();
  dbgs() << "CFI Running on: " << funcName << ".\n";
  auto funcVertex = graph::Vertex(funcName);
  bool first_instr = true;
  if (!F.getMetadata(cfi_guard_str)) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (first_instr) {
          LLVMContext &Ctx = F.getContext();

          FunctionType *registerType =
              TypeBuilder<void(char *), false>::get(Ctx);
          auto registerFunction =
              cast<Function>(F.getParent()->getOrInsertFunction(
                  "registerFunction", registerType));

          IRBuilder<> builder(&I);
          builder.SetInsertPoint(&BB, builder.GetInsertPoint());

          Value *strPtr = builder.CreateGlobalStringPtr(funcName);
          undoValues.insert(builder.CreateCall(registerFunction, strPtr));
          first_instr = false;

          // Function is in the sensitive list
          if (F.hasFnAttribute(CONTROL_FLOW_INTEGRITY)) {
            FunctionType *verifyType = TypeBuilder<void(), false>::get(Ctx);
            auto verifyFunction = cast<Function>(
                F.getParent()->getOrInsertFunction("verifyStack", verifyType));

            // Insert call
            builder.SetInsertPoint(&BB, builder.GetInsertPoint());
            auto call = builder.CreateCall(verifyFunction);
	    setMetadata(Ctx, call, "cfi_verify");
            undoValues.insert(call);
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
            graph->addEdge(funcVertex, calledVertex);
          }
        }
        if (isa<ReturnInst>(&I)) {
          LLVMContext &Ctx = F.getContext();

          FunctionType *registerType =
              TypeBuilder<void(char *), false>::get(Ctx);
          auto deregisterFunction =
              cast<Function>(F.getParent()->getOrInsertFunction(
                  "deregisterFunction", registerType));

          IRBuilder<> builder(&I);
          builder.SetInsertPoint(&BB, builder.GetInsertPoint());

          // Insert a call to our function.
          Value *strPtr = builder.CreateGlobalStringPtr(funcName);
          undoValues.insert(builder.CreateCall(deregisterFunction, strPtr));
        }
      }
    }
  }

  for (auto other : funcAddressTaken) {
    auto calledVertex =
        graph::Vertex(other.first->getName().str(), other.second);
    graph->addEdge(funcVertex, calledVertex);
  }

  return undoValues;
}

void ControlFlowIntegrityPass::getAnalysisUsage(AnalysisUsage &usage) const {
  usage.setPreservesAll();
  usage.addRequired<FunctionFilterPass>();
}

bool ControlFlowIntegrityPass::skip_function(llvm::Function &F,
                                             FunctionInformation *info) {
  if (F.hasAddressTaken()) {
    return true;
  }
  if(F.getName()=="dummyFunc"){
    return true;
  }

  return !info->get_functions().empty() && !info->is_function(&F);
}
} // namespace cfi
