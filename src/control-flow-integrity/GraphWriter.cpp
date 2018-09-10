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
void GraphWriter::write() {
  std::vector<graph::Vertex> paths = getPathsToSensitiveNodes();

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
  std::stringstream ss;
  for (const graph::Edge &e : edgesOnPath) {
    ss << e.str() << "\\n";
  }
  std::string edges = ss.str();

  // Write checksum to file
  rewriteStackAnalysis(edges, verticesOnPath.size(), edgesOnPath.size());
}

void GraphWriter::rewriteStackAnalysis(const std::string &edges, const size_t numVertices, const size_t numEdges) {
  std::ifstream filein(classTemplate); //File to read from
  std::ofstream fileout("NewStackAnalysis.c"); //Temporary file
  if (!filein || !fileout) {
    errs() << "Error opening files!\n";
    return;
  }

  std::regex searchGraphText(R"(\s*char\s*graph_text\s*\[\s*\]\s*=\s*"123"\s*;)");
  std::regex searchVertReplace(R"(\s*\*\s*vertices_count\s*=\s*123\s*;)");
  std::regex searchLineReplace(R"(\s*int\s*line_count\s*=\s*123\s*;)");

  std::string strTemp;
  while (std::getline(filein, strTemp)) {
    if (std::regex_match(strTemp, searchGraphText)) {
      strTemp = "char graph_text[] = \"" + edges + "\";";
    } else if (std::regex_match(strTemp, searchVertReplace)) {
      strTemp = "*vertices_count = " + std::to_string(numVertices) + ";";
    } else if (std::regex_match(strTemp, searchLineReplace)) {
      strTemp = "int line_count = " + std::to_string(numEdges) + ";";
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

GraphWriter::GraphWriter(const graph::Graph &graph, const std::string &classTemplate)
    : graph(graph), classTemplate(classTemplate) {}

void GraphWriter::writeStatsFile(const std::string &filename, const std::vector<graph::Vertex> &registeredVertices) {
  std::ofstream fileout(filename);
  if (!fileout) {
    llvm::errs() << "Error opening stats file\n";
    return;
  }
  fileout << "{\n  \"vertices\": [";
  // loop through vertices
  bool isFirst = true;
  for (const graph::Vertex &v : registeredVertices) {
    if (isFirst) {
      isFirst = false;
    } else {
      fileout << ", ";
    }
    fileout << "\"" + v.getMethodName() + "\"";
  }
  fileout << "],\n  \"sensitiveNodes\": [";
  // loop through sensitive nodes
  isFirst = true;
  for (const graph::Vertex &v : getSensitiveNodes()) {
    if (isFirst) {
      isFirst = false;
    } else {
      fileout << ", ";
    }
    fileout << "\"" + v.getMethodName() + "\"";
  }
  fileout << "]\n}";
}
}