add_library(ControlFlowIntegrityPass MODULE
        # List your source files here.
        FunctionPass.cpp
        graph/Graph.cpp
        FunctionPass.h
        graph/Graph.h graph/Vertex.cpp graph/Vertex.h graph/Edge.cpp graph/Edge.h)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(ControlFlowIntegrityPass PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(ControlFlowIntegrityPass PROPERTIES
        COMPILE_FLAGS "-fno-rtti -lssl -lcrypto"
        )

target_link_libraries(ControlFlowIntegrityPass ${OPENSSL_LIBRARIES})

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if (APPLE)
    set_target_properties(ControlFlowIntegrityPass PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
            )
endif (APPLE)
