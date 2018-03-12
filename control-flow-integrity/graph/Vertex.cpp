#include "Vertex.h"

namespace graph {
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
}