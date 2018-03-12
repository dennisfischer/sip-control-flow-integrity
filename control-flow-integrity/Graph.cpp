#include <vector>
#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "Graph.h"
#include <openssl/sha.h>


Vertex::Vertex(std::string method) {
    methodName = method;
    sensitive = false;
}

Vertex::Vertex(std::string method, bool sensitive) {
    methodName = method;
    this->sensitive = sensitive;
}

Vertex::Vertex() {
    methodName = "";
    sensitive = false;
}

std::string Vertex::getMethodName() { return methodName; }

bool Vertex::isSensitive() { return sensitive; }

Edge::Edge(Vertex org, Vertex dest) {
    origin = org;
    destination = dest;
}

Vertex Edge::getOrigin() { return origin; }

Vertex Edge::getDestination() { return destination; }


Graph::Graph() = default;

void Graph::insert(Vertex v) {
    vertices.push_back(v);
}

bool Graph::contains(Vertex v) {
    return (find(vertices.begin(), vertices.end(), v) != vertices.end());
}

int Graph::addEdge(Vertex origin, Vertex destination) {
    Edge newEdge(origin, destination);
    if (find(vertices.begin(), vertices.end(), origin) == vertices.end()) {
        vertices.push_back(origin);
    } else if (origin.isSensitive()) {
        long index = find(vertices.begin(), vertices.end(), origin) - vertices.begin();
        vertices[index] = origin;
    }
    if (find(vertices.begin(), vertices.end(), destination) == vertices.end()) {
        vertices.push_back(destination);
    } else if (destination.isSensitive()) {
        long index = find(vertices.begin(), vertices.end(), destination) - vertices.begin();
        vertices[index] = destination;
    }
    if (find(edges.begin(), edges.end(), newEdge) == edges.end())
        edges.push_back(newEdge);
}

std::vector<Vertex> Graph::getCallees(Vertex v) {
    std::vector<Vertex> result;
    for (int i = 0; i < edges.size(); i++) {
        if (edges[i].getOrigin() == v) {
            result.push_back(edges[i].getDestination());
        }
    }
    return result;
}

std::vector<Vertex> Graph::getCallers(Vertex v) {
    std::vector<Vertex> result;
    for (int i = 0; i < edges.size(); i++) {
        if (edges[i].getDestination() == v) {
            result.push_back(edges[i].getOrigin());
        }
    }
    return result;
}

Vertex Graph::getFirstNode() {
    for (int i = 0; i < vertices.size(); i++) {
        if (getCallers(vertices[i]).size() == 0) {
            return vertices[i];
        }
    }
}

std::vector<Vertex> Graph::getLastNodes() {
    std::vector<Vertex> result;
    for (int i = 0; i < vertices.size(); i++) {
        if (getCallees(vertices[i]).size() == 0) {
            result.push_back(vertices[i]);
        }
    }
    return result;
}

std::vector<Vertex> Graph::getSensitiveNodes() {
    std::vector<Vertex> result;
    for (Vertex &v : vertices) {
        if (v.isSensitive()) {
            result.push_back(v);
        }
    }
    return result;
}

void rewriteStackAnalysis(std::string checksum) {
    std::ifstream filein("../control-flow-integrity/StackAnalysis.c"); //File to read from
    std::ofstream fileout("../control-flow-integrity/NewStackAnalysis.c"); //Temporary file
    if (!filein || !fileout) {
        std::cout << "Error opening files!" << std::endl;
        return;
    }

    std::string strReplace = "	char *expectedHash = \"";

    std::string strTemp;
    while (getline(filein, strTemp)) {
        if (strTemp.find(strReplace) != std::string::npos) {
            strTemp = "	char *expectedHash = \"" + checksum + "\";";
        }
        strTemp += "\n";
        fileout << strTemp;
    }
}

void Graph::writeGraphFile() {
    std::vector<Vertex> paths = getPathsToSensitiveNodes();
    std:: ofstream outFile;

    outFile.open("graph.txt");
    std::vector<Vertex> verticesOnPath;
    std::vector<Edge> edgesOnPath;
    for (Edge &e : edges) {
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
    sort(edgesOnPath.begin(), edgesOnPath.end(), [](Edge e1, Edge e2) -> bool {
        return e1.getOrigin().str() < e2.getOrigin().str();
    });
    outFile << verticesOnPath.size() << std::endl;
    outFile << edgesOnPath.size() << std::endl;
    for (Edge &e : edgesOnPath)
        outFile << e.str() << "\n";
    outFile.close();

    unsigned char c[SHA256_DIGEST_LENGTH];
    FILE *inFile = fopen("graph.txt", "rb");
    if (inFile == nullptr) {
        printf("graph.txt can't be opened.\n");
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
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::setfill('0') << std::setw(2) << std::hex << (int) c[i];

    std::string checksum = ss.str();

    // Write checksum to file
    rewriteStackAnalysis(checksum);
}

std::vector<Vertex> Graph::getPathsToSensitiveNodes() {
    std::vector<Vertex> pathToSensitiveFunctions = getSensitiveNodes();
    size_t size = pathToSensitiveFunctions.size();
    for (int i = 0; i < size; i++) {
        std::vector<Vertex> newDominators = getCallers(pathToSensitiveFunctions[i]);
        for (auto it = newDominators.begin(); it != newDominators.end(); ++it) {
            if (find(pathToSensitiveFunctions.begin(), pathToSensitiveFunctions.end(), *it)
                == pathToSensitiveFunctions.end()) {
                pathToSensitiveFunctions.push_back(*it);
                size++;
            }
        }
    }
    return pathToSensitiveFunctions;
}
