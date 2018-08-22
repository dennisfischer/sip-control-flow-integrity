#include <vector>
#include <unordered_set>
#include <set>
#include <control-flow-integrity/GraphWriter.h>
#include <control-flow-integrity/CFIAnalysis.h>

using namespace llvm;

namespace cfi {
static RegisterPass<ControlFlowIntegrityGraphPass>
    X("control-flow-integrity", "Control Flow Integrity Graph Writer Pass", false, false);

cl::opt<std::string> StackAnalysisTemplate
    ("cfi-template", cl::Hidden, cl::desc("File path to the source file template used for the StackAnalysis"));

char ControlFlowIntegrityGraphPass::ID = 0;

void ControlFlowIntegrityGraphPass::writeGraphFile(const graph::Graph &graph) {
  std::vector<graph::Vertex> paths = getPathsToSensitiveNodes();
  std::ofstream outFile;

  std::unordered_set<graph::Vertex> verticesOnPath;
  std::set<graph::Edge> edgesOnPath;
  for (auto &e : graph.getEdges()) {
    // Only add edges that are contained in a path to a sensitive function to get a
    // smaller adjacency matrix in the stack analysis
    if (std::find(paths.begin(), paths.end(), e.getDestination()) != paths.end()
        && std::find(paths.begin(), paths.end(), e.getOrigin()) != paths.end()) {
      edgesOnPath.insert(e);
      if (verticesOnPath.find(e.getOrigin()) == verticesOnPath.end())
        verticesOnPath.insert(e.getOrigin());
      if (verticesOnPath.find(e.getDestination()) == verticesOnPath.end())
        verticesOnPath.insert(e.getDestination());
    }
  }

  outFile.open("graph.txt");
  outFile << verticesOnPath.size() << std::endl;
  outFile << edgesOnPath.size() << std::endl;
  for (const auto &e : edgesOnPath) {
    outFile << e.str() << "\n";
  }
  outFile.close();

  dbgs() << "Writing file graph.txt\n";
  FILE *inFile = fopen("graph.txt", "rb");
  if (inFile == nullptr) {
    errs() << "graph.txt cannot be opened.\n";
    return;
  }

  std::string checksum = hashFile(inFile);
  fclose(inFile);

  dbgs() << "Checksum is: ";
  dbgs() << checksum;
  dbgs() << "\n";

  // Write checksum to file
  rewriteStackAnalysis(checksum);
}

std::string ControlFlowIntegrityGraphPass::hashFile(FILE *inFile) const {
  unsigned char c[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  size_t bytes;
  unsigned char data[1024];
  SHA256_Init(&sha256);
  while ((bytes = fread(data, 1, 1024, inFile)) != 0) {
    SHA256_Update(&sha256, data, bytes);
  }

  SHA256_Final(c, &sha256);

  std::stringstream ss;
  for (unsigned char i : c) {
    ss << std::setfill('0') << std::setw(2) << std::hex << (int) i;
  }

  return ss.str();
}

void ControlFlowIntegrityGraphPass::rewriteStackAnalysis(const std::string &checksum) {
  std::ifstream filein(StackAnalysisTemplate.getValue()); //File to read from
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

std::vector<graph::Vertex> ControlFlowIntegrityGraphPass::getPathsToSensitiveNodes() {
  std::vector<graph::Vertex> pathToSensitiveFunctions = getSensitiveNodes();
  size_t size = pathToSensitiveFunctions.size();
  for (int i = 0; i < size; i++) {
    std::vector<graph::Vertex> newDominators = getCallers(pathToSensitiveFunctions[i]);
    for (auto &newDominator : newDominators) {
      if (std::find(pathToSensitiveFunctions.begin(), pathToSensitiveFunctions.end(), newDominator)
          == pathToSensitiveFunctions.end()) {
        pathToSensitiveFunctions.push_back(newDominator);
        size++;
      }
    }
  }
  return pathToSensitiveFunctions;
}

std::vector<graph::Vertex> ControlFlowIntegrityGraphPass::getCallers(const graph::Vertex &v) {
  std::vector<graph::Vertex> result;
  auto edges = graph.getEdges();

  for (auto &edge : edges) {
    if (edge.getDestination() == v) {
      result.push_back(edge.getOrigin());
    }
  }
  return result;
}

std::vector<graph::Vertex> ControlFlowIntegrityGraphPass::getCallees(const graph::Vertex &v) {
  std::vector<graph::Vertex> result;
  auto edges = graph.getEdges();

  for (auto &edge : edges) {
    if (edge.getOrigin() == v) {
      result.push_back(edge.getDestination());
    }
  }
  return result;
}

std::vector<graph::Vertex> ControlFlowIntegrityGraphPass::getSensitiveNodes() {
  std::vector<graph::Vertex> result;
  for (graph::Vertex &v : graph.getVertices()) {
    if (v.isSensitive()) {
      result.push_back(v);
    }
  }
  return result;
}

bool ControlFlowIntegrityGraphPass::runOnModule(llvm::Module &M) {
  graph = getAnalysis<ControlFlowIntegrityPass>().getGraph();
  dbgs() << "Finalizing...\n";
  writeGraphFile(graph);
  return false;
}

void ControlFlowIntegrityGraphPass::getAnalysisUsage(llvm::AnalysisUsage &usage) const {
  usage.setPreservesAll();
  usage.addRequiredTransitive<ControlFlowIntegrityPass>();
}
}