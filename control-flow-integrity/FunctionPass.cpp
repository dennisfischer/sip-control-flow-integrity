#include "FunctionPass.h"

using namespace cfi;
using namespace llvm;

char ControlFlowIntegrityPass::ID = 0;

bool ControlFlowIntegrityPass::doInitialization(Module &M) {
	errs() << "Control Flow Integrity pass loaded...\n";
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
				errs() << "Sensitive function: " << fn->getName().str() << "\n";
			}
		}
	}
	// End annotations from @ http://bholt.org/posts/llvm-quick-tricks.html
	return true;
}

std::vector<graph::Vertex> ControlFlowIntegrityPass::getCallers(const graph::Vertex &v) {
	std::vector<graph::Vertex> result;
	auto edges = graph.getEdges();

	for (auto &edge : edges) {
		if (edge.getDestination() == v) {
			result.push_back(edge.getOrigin());
		}
	}
	return result;
}

std::vector<graph::Vertex> ControlFlowIntegrityPass::getPathsToSensitiveNodes() {
	std::vector<graph::Vertex> pathToSensitiveFunctions = getSensitiveNodes();
	size_t size = pathToSensitiveFunctions.size();
	for (int i = 0; i < size; i++) {
		std::vector<graph::Vertex> newDominators = getCallers(pathToSensitiveFunctions[i]);
		for (auto &newDominator : newDominators) {
			if (find(pathToSensitiveFunctions.begin(), pathToSensitiveFunctions.end(), newDominator)
			    == pathToSensitiveFunctions.end()) {
				pathToSensitiveFunctions.push_back(newDominator);
				size++;
			}
		}
	}
	return pathToSensitiveFunctions;
}

std::vector<graph::Vertex> ControlFlowIntegrityPass::getCallees(const graph::Vertex &v) {
	std::vector<graph::Vertex> result;
	auto edges = graph.getEdges();

	for (auto &edge : edges) {
		if (edge.getOrigin() == v) {
			result.push_back(edge.getDestination());
		}
	}
	return result;
}

std::vector<graph::Vertex> ControlFlowIntegrityPass::getSensitiveNodes() {
	std::vector<graph::Vertex> result;
	for (graph::Vertex &v : graph.getVertices()) {
		if (v.isSensitive()) {
			result.push_back(v);
		}
	}
	return result;
}

void ControlFlowIntegrityPass::rewriteStackAnalysis(const std::string &checksum) {
	std::ifstream filein("/home/dennis/sip-control-flow-integrity/control-flow-integrity/stack_analysis/StackAnalysis.c"); //File to read from
	std::ofstream fileout("NewStackAnalysis.c"); //Temporary file
	if (!filein || !fileout) {
		errs() << "Error opening files!\n";
		return;
	}

	std::regex search(R"(\s*char\s*\*\s*expectedHash\s*=\s*"123"\s*;)");

	std::string strTemp;
	while (std::getline(filein, strTemp)) {
		if (std::regex_match(strTemp, search)) {
			strTemp = "char *expectedHash = \"" + checksum + "\";";
		}
		strTemp += "\n";
		fileout << strTemp;
	}
}

void ControlFlowIntegrityPass::writeGraphFile(const graph::Graph &graph) {
	std::vector<graph::Vertex> paths = getPathsToSensitiveNodes();
	std::ofstream outFile;

	outFile.open("graph.txt");
	std::vector<graph::Vertex> verticesOnPath;
	std::vector<graph::Edge> edgesOnPath;
	for (graph::Edge &e : graph.getEdges()) {
		// Only add edges that are contained in a path to a sensitive function to get a
		// smaller adjacency matrix in the stack analysis
		if (find(paths.begin(), paths.end(), e.getDestination()) != paths.end()
		    && find(paths.begin(), paths.end(), e.getOrigin()) != paths.end()) {
			edgesOnPath.push_back(e);
			if (find(verticesOnPath.begin(), verticesOnPath.end(), e.getOrigin()) == verticesOnPath.end())
				verticesOnPath.push_back(e.getOrigin());
			if (find(verticesOnPath.begin(), verticesOnPath.end(), e.getDestination()) == verticesOnPath.end())
				verticesOnPath.push_back(e.getDestination());
		}
	}
	sort(edgesOnPath.begin(), edgesOnPath.end(), [](graph::Edge e1, graph::Edge e2) -> bool {
		return e1.getOrigin().str() < e2.getOrigin().str();
	});
	outFile << verticesOnPath.size() << std::endl;
	outFile << edgesOnPath.size() << std::endl;
	for (graph::Edge &e : edgesOnPath)
		outFile << e.str() << "\n";
	outFile.close();

	unsigned char c[SHA256_DIGEST_LENGTH];
	FILE *inFile = fopen("graph.txt", "rb");
	if (inFile == nullptr) {
		errs() << "graph.txt cannot be opened.\n";
		return;
	}
	SHA256_CTX sha256;
	size_t bytes;
	unsigned char data[1024];
	SHA256_Init(&sha256);
	while ((bytes = fread(data, 1, 1024, inFile)) != 0)
		SHA256_Update(&sha256, data, bytes);
	fclose(inFile);

	SHA256_Final(c, &sha256);

	std::stringstream ss;
	for (unsigned char i : c) {
		ss << std::setfill('0') << std::setw(2) << std::hex << (int) i;
	}

	std::string checksum = ss.str();
	errs() << "Checksum is: ";
	errs() << checksum;
	errs() << "\n";

	// Write checksum to file
	rewriteStackAnalysis(checksum);
}

bool ControlFlowIntegrityPass::runOnFunction(Function &function) {
	std::string funcName = function.getName().str();
	graph::Vertex funcVertex = graph::Vertex(funcName);
	bool first_instr = true;
	bool modified = false; // runOnFunction return value
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
	return modified;
}

bool ControlFlowIntegrityPass::doFinalization(Module &M) {
	errs() << "Finalizing...\n";
	writeGraphFile(graph);
	return false;
}

static RegisterPass<ControlFlowIntegrityPass> X("control-flow-integrity", "Control Flow Integrity Pass", false, false);

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerCFIPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
	PM.add(new ControlFlowIntegrityPass());
}

static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerCFIPass);
