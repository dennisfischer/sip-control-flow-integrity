add_library(StackAnalysis SHARED
        # List your source files here.
        StackAnalysis.c
        StackAnalysis.h
        #StackAnalysis.cpp
        #StackAnalysis.hpp
        )

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(StackAnalysis PRIVATE cxx_range_for cxx_auto_type)

target_link_libraries(StackAnalysis ${OPENSSL_LIBRARIES})
