#ifndef PROJECT_VERTEX_H
#define PROJECT_VERTEX_H

#include <vector>
#include <string>

namespace graph {

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
}
#endif //PROJECT_VERTEX_H
