#include "Edge.h"

namespace graph {

    Edge::Edge(Vertex org, Vertex dest) {
        origin = org;
        destination = dest;
    }

    Vertex Edge::getOrigin() { return origin; }

    Vertex Edge::getDestination() { return destination; }
}