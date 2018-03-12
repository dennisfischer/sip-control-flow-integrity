cmake_minimum_required(VERSION 3.1)

add_library(graph SHARED Graph.cpp Graph.h Vertex.cpp Vertex.h Edge.cpp Edge.h)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})