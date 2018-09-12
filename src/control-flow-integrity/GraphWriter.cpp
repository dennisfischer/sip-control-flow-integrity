#include <utility>
#include <vector>
#include <unordered_set>
#include <set>
#include <iomanip>
#include <fstream>
#include <regex>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <openssl/sha.h>
#include <control-flow-integrity/GraphWriter.h>

using namespace llvm;

namespace cfi {
GraphWriter::GraphWriter(const graph::Graph &graph) : graph(graph) {}

void GraphWriter::write(const std::string &outPath, const std::string &classTemplate) {
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

  std::string filePath = outPath + "/graph.txt";
  outFile.open(filePath);
  outFile << verticesOnPath.size() << std::endl;
  outFile << edgesOnPath.size() << std::endl;
  for (const auto &e : edgesOnPath) {
    outFile << e.str() << "\n";
  }
  outFile.close();

  dbgs() << "Writing file graph.txt\n";
  FILE *inFile = fopen(filePath.c_str(), "rb");
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
  rewriteStackAnalysis(checksum, outPath, classTemplate);
}

std::string GraphWriter::hashFile(FILE *inFile) const {
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

void GraphWriter::rewriteStackAnalysis(const std::string &checksum, const std::string &outPath, const std::string &classTemplate) {
  std::ifstream filein(classTemplate); //File to read from
  std::ofstream fileout(outPath+"/NewStackAnalysis.c"); //Temporary file
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

std::vector<graph::Vertex> GraphWriter::getPathsToSensitiveNodes() {
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

std::vector<graph::Vertex> GraphWriter::getCallers(const graph::Vertex &v) {
  std::vector<graph::Vertex> result;
  auto edges = graph.getEdges();

  for (auto &edge : edges) {
    if (edge.getDestination() == v) {
      result.push_back(edge.getOrigin());
    }
  }
  return result;
}

std::vector<graph::Vertex> GraphWriter::getCallees(const graph::Vertex &v) {
  std::vector<graph::Vertex> result;
  auto edges = graph.getEdges();

  for (auto &edge : edges) {
    if (edge.getOrigin() == v) {
      result.push_back(edge.getDestination());
    }
  }
  return result;
}

std::vector<graph::Vertex> GraphWriter::getSensitiveNodes() {
  std::vector<graph::Vertex> result;
  for (graph::Vertex &v : graph.getVertices()) {
    if (v.isSensitive()) {
      result.push_back(v);
    }
  }
  return result;
}
}