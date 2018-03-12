// clang-5.0 -emit-llvm something.c -c something.bc
// mkdir build; cd build
// cmake ..
// make
// opt-5.0 -load control-flow-integrity/libFunctionPass.so -functionpass < ../something.bc > /dev/null

#include "FunctionPass.h"

using namespace llvm;

namespace {
    struct ControlFlowIntegrityPass : public FunctionPass {
        static char ID;
        graph::Graph graph;

        ControlFlowIntegrityPass() : FunctionPass(ID) {}

        bool doInitialization(Module &M) override {
            graph = graph::Graph();

            // Start annotations from @ http://bholt.org/posts/llvm-quick-tricks.html
            auto global_annos = M.getNamedGlobal("llvm.global.annotations");
            if (global_annos) {
                auto a = cast<ConstantArray>(global_annos->getOperand(0));
                for (unsigned int i = 0; i < a->getNumOperands(); i++) {
                    auto e = cast<ConstantStruct>(a->getOperand(i));

                    if (auto fn = dyn_cast<Function>(e->getOperand(0)->getOperand(0))) {
                        auto anno = cast<ConstantDataArray>(
                                cast<GlobalVariable>(e->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
                        fn->addFnAttr(anno); // <-- add function annotation here
                        errs() << "Sensitive function: " << fn->getName().str() << "\n";
                    }
                }
            }
            // End annotations from @ http://bholt.org/posts/llvm-quick-tricks.html
            return true;
        }

        bool doFinalization(Module &M) override {
            graph.writeGraphFile();
            return false;
        }

        bool runOnFunction(Function &function) override {
            std::string funcName = function.getName().str();
            graph::Vertex funcVertex = graph::Vertex(funcName);
            bool first_instr = true;
            bool modified = false; // runOnFunction return value
            for (BasicBlock &block : function) {
                for (Instruction &instruction: block) {
                    if (first_instr) {
                        LLVMContext &Ctx = function.getContext();

                        FunctionType *registerType = TypeBuilder<void(char *), false>::get(Ctx);
                        Function *registerFunction = cast<Function>(function.getParent()->
                                getOrInsertFunction("registerFunction", registerType));

                        IRBuilder<> builder(&instruction);
                        builder.SetInsertPoint(&block, builder.GetInsertPoint());

                        Value *strPtr = builder.CreateGlobalStringPtr(funcName.c_str());
                        builder.CreateCall(registerFunction, strPtr);
                        modified = true;
                        first_instr = false;

                        // Function is in the sensitive list
                        if (function.hasFnAttribute(CONTROL_FLOW_INTEGRITY)) {
                            FunctionType *verifyType = TypeBuilder<void(), false>::get(Ctx);
                            Function *verifyFunction = cast<Function>(function.getParent()->
                                    getOrInsertFunction("verifyStack", verifyType));

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
                        Function *deregisterFunction = cast<Function>(
                                function.getParent()->getOrInsertFunction("deregisterFunction", registerType)
                        );

                        IRBuilder<> builder(&instruction);
                        builder.SetInsertPoint(&block, builder.GetInsertPoint());

                        // Insert a call to our function.
                        Value *strPtr = builder.CreateGlobalStringPtr(funcName.c_str());
                        builder.CreateCall(deregisterFunction, strPtr);
                        modified = true;
                    }
                }
            }
            return modified;
        }
    };
}
char ControlFlowIntegrityPass::ID = 0;
static RegisterPass<ControlFlowIntegrityPass> X("control_flow_integrity", "Control Flow Integrity Pass", false, false);

/*
static RegisterStandardPasses ControlFlowIntegrityPassRegistration(PassManagerBuilder::EP_EarlyAsPossible,
                                                                   [](const PassManagerBuilder &,
                                                                      legacy::PassManagerBase &PM) {
                                                                       errs() << "Registered pass!\n";
                                                                       PM.add(new ControlFlowIntegrityPass());
                                                                   });*/
