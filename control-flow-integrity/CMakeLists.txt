project(cfi_pass)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

if ($ENV{CLION_IDE})
    include_directories("/usr/include/llvm-6.0/")
endif ()

add_library(ControlFlowIntegrityPass MODULE
        # List your source files here.
        FunctionPass.cpp
        FunctionPass.h)

target_link_libraries(ControlFlowIntegrityPass StackAnalysis graph ${OPENSSL_LIBRARIES})


add_subdirectory(stack_analysis)
add_subdirectory(graph)
# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(ControlFlowIntegrityPass PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(ControlFlowIntegrityPass PROPERTIES
        COMPILE_FLAGS "-fno-rtti -lssl -lcrypto"
        )


# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if (APPLE)
    set_target_properties(ControlFlowIntegrityPass PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
            )
endif (APPLE)
