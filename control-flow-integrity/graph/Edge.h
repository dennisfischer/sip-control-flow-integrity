#ifndef PROJECT_EDGE_H
#define PROJECT_EDGE_H

#include <sstream>
#include "Vertex.h"

namespace graph {

	class Edge {
	public:
		Edge(Vertex org, Vertex dest);

		Vertex getOrigin() const;

		Vertex getDestination() const;

		bool operator==(const Edge &rhs) const {
			return (origin == rhs.origin) && (destination == rhs.destination);
		};

		std::string str() const {
			std::ostringstream os;
			os << this->origin.getMethodName() << " " << this->destination.getMethodName();
			return os.str();
		}

		bool operator<(const Edge &other) const {
			return this->str() < other.str();
		}

	private:
		Vertex origin;
		Vertex destination;
	};
}
namespace std {
	template<>
	struct hash<graph::Edge> {
		size_t operator()(const graph::Edge &x) const {
			return (hash<graph::Vertex>()(x.getOrigin()) << 1) ^ hash<graph::Vertex>()(x.getDestination());
		}
	};
}
#endif //PROJECT_EDGE_H
