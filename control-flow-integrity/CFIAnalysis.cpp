#include "CFIAnalysis.h"

using namespace cfi;
using namespace llvm;

char ControlFlowIntegrityPass::ID = 0;

bool ControlFlowIntegrityPass::doInitialization(Module &M) {
	dbgs() << "Control Flow Integrity pass loaded...\n";
	graph = graph::Graph();

	// Start annotations from @ http://bholt.org/posts/llvm-quick-tricks.html
	auto global_annos = M.getNamedGlobal("llvm.global.annotations");
	if (global_annos) {
		auto a = cast<ConstantArray>(global_annos->getOperand(0));
		for (unsigned int i = 0; i < a->getNumOperands(); i++) {
			auto e = cast<ConstantStruct>(a->getOperand(i));

			if (auto fn = dyn_cast<Function>(e->getOperand(0)->getOperand(0))) {
				auto anno = cast<ConstantDataArray>(cast<GlobalVariable>(e->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
				fn->addFnAttr(anno); // <-- add function annotation here
				dbgs() << "Sensitive function: " << fn->getName().str() << "\n";
			}
		}
	}
	// End annotations from @ http://bholt.org/posts/llvm-quick-tricks.html
	return true;
}

bool ControlFlowIntegrityPass::runOnModule(Module &M) {
	bool modified = false; // runOnFunction return value

	for (auto &function : M) {
		std::string funcName = function.getName().str();
		dbgs() << "Running on: " << funcName << ".\n";
		graph::Vertex funcVertex = graph::Vertex(funcName);
		bool first_instr = true;
		for (BasicBlock &block : function) {
			for (Instruction &instruction: block) {
				if (first_instr) {
					LLVMContext &Ctx = function.getContext();

					FunctionType *registerType = TypeBuilder<void(char *), false>::get(Ctx);
					auto registerFunction = cast<Function>(function.getParent()->getOrInsertFunction("registerFunction", registerType));

					IRBuilder<> builder(&instruction);
					builder.SetInsertPoint(&block, builder.GetInsertPoint());

					Value *strPtr = builder.CreateGlobalStringPtr(funcName);
					builder.CreateCall(registerFunction, strPtr);
					modified = true;
					first_instr = false;

					// Function is in the sensitive list
					if (function.hasFnAttribute(CONTROL_FLOW_INTEGRITY)) {
						FunctionType *verifyType = TypeBuilder<void(), false>::get(Ctx);
						auto verifyFunction = cast<Function>(function.getParent()->getOrInsertFunction("verifyStack", verifyType));

						// Insert call
						builder.SetInsertPoint(&block, builder.GetInsertPoint());
						builder.CreateCall(verifyFunction);
					}
				}
				if (auto *callInstruction = dyn_cast<CallInst>(&instruction)) {
					Function *called = callInstruction->getCalledFunction();
					if (called) {
						std::string calledName = called->getName().str();
						graph::Vertex calledVertex;
						if (called->hasFnAttribute(CONTROL_FLOW_INTEGRITY)) {
							calledVertex = graph::Vertex(calledName, true);
						} else {
							calledVertex = graph::Vertex(calledName);
						}
						graph.addEdge(funcVertex, calledVertex);
					}
				}
				if (auto *callInstruction = dyn_cast<ReturnInst>(&instruction)) {
					LLVMContext &Ctx = function.getContext();

					FunctionType *registerType = TypeBuilder<void(char *), false>::get(Ctx);
					auto deregisterFunction = cast<Function>(function.getParent()->getOrInsertFunction("deregisterFunction", registerType));

					IRBuilder<> builder(&instruction);
					builder.SetInsertPoint(&block, builder.GetInsertPoint());

					// Insert a call to our function.
					Value *strPtr = builder.CreateGlobalStringPtr(funcName);
					builder.CreateCall(deregisterFunction, strPtr);
					modified = true;
				}
			}
		}
	}
	if (modified) {
		dbgs() << "############## MODIFIED #############\n";
	}
	return modified;
}

void ControlFlowIntegrityPass::getAnalysisUsage(AnalysisUsage &usage) const {
	usage.setPreservesAll();
}

static RegisterPass<ControlFlowIntegrityPass> X("control-flow-integrity-analysis", "Control Flow Integrity Analysis Pass");