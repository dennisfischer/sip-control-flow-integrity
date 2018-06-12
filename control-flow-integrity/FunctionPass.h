#ifndef CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H
#define CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H

#include <cstdio>
#include <fstream>
#include <sstream>
#include <fstream>
#include <iostream>
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

#ifndef CONTROL_FLOW_INTEGRITY
#define CONTROL_FLOW_INTEGRITY "control_flow_integrity"
#endif

namespace cfi {
	class ControlFlowIntegrityPass : public llvm::FunctionPass {
	public:
		static char ID;
	private:
		graph::Graph graph;
	public:
		ControlFlowIntegrityPass() : FunctionPass(ID) {}

		bool doInitialization(llvm::Module &M) override;

		bool doFinalization(llvm::Module &M) override;

		bool runOnFunction(llvm::Function &function) override;

	private:
		void writeGraphFile(const graph::Graph &graph);

		void rewriteStackAnalysis(const std::string &checksum);

		std::vector<graph::Vertex> getSensitiveNodes();

		std::vector<graph::Vertex> getPathsToSensitiveNodes();

		std::vector<graph::Vertex> getCallees(const graph::Vertex &v);

		std::vector<graph::Vertex> getCallers(const graph::Vertex &v);
	};
}

#endif //CONTROL_FLOW_INTEGRITY_FUNCTION_PASS_H
