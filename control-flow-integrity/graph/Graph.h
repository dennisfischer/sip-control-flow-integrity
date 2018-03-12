#pragma once

#ifndef PROJECT_GRAPH_H
#define PROJECT_GRAPH_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <openssl/sha.h>

#include "Vertex.h"
#include "Edge.h"

namespace graph {
    class Graph {
    public:
        Graph();

        bool contains(Vertex v);

        void insert(Vertex v);

        void addEdge(Vertex origin, Vertex destination);

        std::vector<Vertex> getCallees(Vertex v);

        std::vector<Vertex> getCallers(Vertex v);

        Vertex getFirstNode();

        std::vector<Vertex> getLastNodes();

        std::vector<Vertex> getSensitiveNodes();

        void writeGraphFile();

        std::vector<Vertex> getPathsToSensitiveNodes();

        std::string str() {
            std::ostringstream os;
            for (Edge e : this->edges)
                os << e.str() << std::endl;
            return os.str();
        }

    private:
        std::vector<Vertex> vertices;
        std::vector<Edge> edges;
    };

}

#endif // PROJECT_GRAPH_H
