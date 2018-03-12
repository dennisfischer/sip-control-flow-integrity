#ifndef PROJECT_EDGE_H
#define PROJECT_EDGE_H

#include <sstream>
#include "Vertex.h"

namespace graph {

    class Edge {
    public:
        Edge(Vertex org, Vertex dest);

        Vertex getOrigin();

        Vertex getDestination();

        bool operator==(const Edge &rhs) const {
            return (origin == rhs.origin) && (destination == rhs.destination);
        };

        std::string str() {
            std::ostringstream os;
            os << this->origin.str() << " " << this->destination.str();
            return os.str();
        }

    private:
        Vertex origin;
        Vertex destination;
    };
}

#endif //PROJECT_EDGE_H
