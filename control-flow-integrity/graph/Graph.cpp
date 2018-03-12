#include "Graph.h"

namespace graph {

    Graph::Graph() = default;

    void Graph::insert(Vertex v) {
        vertices.push_back(v);
    }

    bool Graph::contains(Vertex v) {
        return (std::find(vertices.begin(), vertices.end(), v) != vertices.end());
    }

    void Graph::addEdge(Vertex origin, Vertex destination) {
        Edge newEdge(origin, destination);
        if (std::find(vertices.begin(), vertices.end(), origin) == vertices.end()) {
            vertices.push_back(origin);
        } else if (origin.isSensitive()) {
            long index = std::find(vertices.begin(), vertices.end(), origin) - vertices.begin();
            vertices[index] = origin;
        }
        if (std::find(vertices.begin(), vertices.end(), destination) == vertices.end()) {
            vertices.push_back(destination);
        } else if (destination.isSensitive()) {
            long index = std::find(vertices.begin(), vertices.end(), destination) - vertices.begin();
            vertices[index] = destination;
        }
        if (std::find(edges.begin(), edges.end(), newEdge) == edges.end())
            edges.push_back(newEdge);
    }

    std::vector<Vertex> Graph::getVertices() const {
        return std::vector<Vertex>(vertices);
    }

    std::vector<Edge> Graph::getEdges() const {
        return std::vector<Edge>(edges);
    }
}