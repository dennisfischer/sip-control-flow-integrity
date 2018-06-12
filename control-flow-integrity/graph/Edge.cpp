#include <utility>

#include "Edge.h"

namespace graph {

	Edge::Edge(Vertex org, Vertex dest) {
		origin = std::move(org);
		destination = std::move(dest);
	}

	Vertex Edge::getOrigin() { return origin; }

	Vertex Edge::getDestination() { return destination; }
}