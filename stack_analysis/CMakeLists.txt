cmake_minimum_required(VERSION 3.12)

project(stack_analysis VERSION 0.1 LANGUAGES C CXX)

find_package(OpenSSL REQUIRED)

add_library(StackAnalysis SHARED
        StackAnalysis.c
        #StackAnalysis.cpp
        )

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(StackAnalysis PRIVATE cxx_std_17)

target_link_libraries(StackAnalysis ${OPENSSL_LIBRARIES})

target_include_directories(StackAnalysis
        PRIVATE
        ${OPENSSL_INCLUDE_DIR}
        )
