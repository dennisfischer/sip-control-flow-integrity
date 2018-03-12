#pragma once

#ifndef PROJECT_GRAPH_H
#define PROJECT_GRAPH_H

#include <vector>
#include <algorithm>
#include "Vertex.h"
#include "Edge.h"

namespace graph {
    class Graph {
    public:
        Graph();

        bool contains(Vertex v);

        void insert(Vertex v);

        void addEdge(Vertex origin, Vertex destination);

        std::string str() {
            std::ostringstream os;
            for (Edge e : this->edges)
                os << e.str() << std::endl;
            return os.str();
        }

        std::vector<Vertex> getVertices() const;

        std::vector<Edge> getEdges() const;

    private:
        std::vector<Vertex> vertices;
        std::vector<Edge> edges;
    };

}

#endif // PROJECT_GRAPH_H
