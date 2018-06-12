find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

add_library(StackAnalysis SHARED
            StackAnalysis.c
            #StackAnalysis.cpp
            )

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(StackAnalysis PRIVATE cxx_range_for cxx_auto_type)

target_link_libraries(StackAnalysis ${OPENSSL_LIBRARIES})
