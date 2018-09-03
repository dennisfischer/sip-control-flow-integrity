cmake_minimum_required(VERSION 3.12)

project(control-flow-integrity VERSION 0.1 LANGUAGES CXX)

add_library(Graph
        include/control-flow-integrity/graph/Graph.h
        include/control-flow-integrity/graph/Vertex.h
        include/control-flow-integrity/graph/Edge.h

        src/control-flow-integrity/graph/Graph.cpp
        src/control-flow-integrity/graph/Vertex.cpp
        src/control-flow-integrity/graph/Edge.cpp)

add_library(ControlFlowIntegrity MODULE
        include/control-flow-integrity/CFIAnalysis.h
        include/control-flow-integrity/GraphWriter.h

        src/control-flow-integrity/CFIAnalysis.cpp
        src/control-flow-integrity/GraphWriter.cpp)

add_library(control-flow-integrity::Graph ALIAS Graph)
add_library(control-flow-integrity::ControlFlowIntegrity ALIAS ControlFlowIntegrity)

find_package(OpenSSL REQUIRED)
find_package(LLVM 6.0 REQUIRED CONFIG)
find_package(composition-framework REQUIRED)
find_package(self-checksumming REQUIRED)

target_include_directories(Graph
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_include_directories(ControlFlowIntegrity
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${INPUT_DEP_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR})
if ($ENV{CLION_IDE})
    include_directories("/usr/include/llvm-6.0/")
    include_directories("/usr/include/llvm-c-6.0/")
endif ()


target_link_libraries(Graph PRIVATE ${OPENSSL_LIBRARIES})
target_link_libraries(ControlFlowIntegrity PRIVATE control-flow-integrity::Graph)

target_compile_features(Graph PRIVATE cxx_std_17)
target_compile_features(ControlFlowIntegrity PRIVATE cxx_std_17)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
target_compile_options(Graph PRIVATE -fno-rtti -fPIC)
target_compile_options(ControlFlowIntegrity PRIVATE -fno-rtti)

add_subdirectory(stack_analysis)