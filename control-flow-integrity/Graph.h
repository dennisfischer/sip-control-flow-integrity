#include <sstream>

class Vertex {
public:
    explicit Vertex(std::string method);

    Vertex(std::string method, bool sensitive);

    Vertex();

    std::string getMethodName();

    bool isSensitive();

    bool operator==(const Vertex &rhs) const {
        return (methodName == rhs.methodName);
    };

    bool operator!=(const Vertex &rhs) const {
        return (methodName != rhs.methodName);
    };

    std::string str() {
        return this->methodName;
    }

private:
    std::string methodName;
    bool sensitive;
};


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
