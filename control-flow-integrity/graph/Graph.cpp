#include "Graph.h"

namespace graph {

	Graph::Graph() = default;

	void Graph::insert(Vertex v) {
		vertices.push_back(v);
	}

	void Graph::insert(Edge e) {
		edges.push_back(e);
	}

	bool Graph::contains(Vertex v) {
		return (std::find(vertices.begin(), vertices.end(), v) != vertices.end());
	}

	bool Graph::contains(Edge e) {
		return (std::find(edges.begin(), edges.end(), e) != edges.end());
	}

	void Graph::addEdge(Vertex origin, Vertex destination) {
		Edge newEdge(origin, destination);
		if (!contains(origin)) {
			insert(origin);
		} else if (origin.isSensitive()) {
			long index = std::find(vertices.begin(), vertices.end(), origin) - vertices.begin();
			vertices[index] = origin;
		}

		if (!contains(destination)) {
			insert(destination);
		} else if (destination.isSensitive()) {
			long index = std::find(vertices.begin(), vertices.end(), destination) - vertices.begin();
			vertices[index] = destination;
		}

		if (!contains(newEdge)) {
			insert(newEdge);
		}
	}

	std::vector<Vertex> Graph::getVertices() const {
		return std::vector<Vertex>(vertices);
	}

	std::vector<Edge> Graph::getEdges() const {
		return std::vector<Edge>(edges);
	}
}