#ifndef PROJECT_VERTEX_H
#define PROJECT_VERTEX_H

#include <vector>
#include <string>

namespace graph {

	class Vertex {
	public:
		explicit Vertex(const std::string &method);

		Vertex(const std::string &method, bool sensitive);

		Vertex();

		std::string getMethodName() const;

		bool isSensitive();

		bool operator==(const Vertex &rhs) const {
			return (methodName == rhs.methodName);
		};

		bool operator!=(const Vertex &rhs) const {
			return (methodName != rhs.methodName);
		};


	private:
		std::string methodName;
		bool sensitive;
	};
}

namespace std {
	template<>
	struct hash<graph::Vertex> {
		size_t operator()(const graph::Vertex &x) const {
			return hash<std::string>()(x.getMethodName());
		}
	};
}
#endif //PROJECT_VERTEX_H
