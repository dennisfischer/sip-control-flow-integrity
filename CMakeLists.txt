cmake_minimum_required(VERSION 3.1)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

if ($ENV{CLION_IDE})
    include_directories("/usr/include/llvm-5.0/")
endif ()

add_subdirectory(control-flow-integrity)  # Use your pass name here.
