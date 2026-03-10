cmake_minimum_required(VERSION 3.20)
project(asan_and_gdb_tutorial C)

set(CMAKE_C_STANDARD 99)

add_executable(c_error_examples c_error_examples.c)

if(MSVC)
    # MSVC Flags: /fsanitize=address enables ASan, /Zi adds debug info
    target_compile_options(c_error_examples PRIVATE /fsanitize=address /Zi /Od)
    target_link_options(c_error_examples PRIVATE /fsanitize=address)
    
    # REQUIRED for MSVC: Disables incremental linking (incompatible with ASan)
    # and ensures we use the correct runtime library.
    set_target_properties(c_error_examples PROPERTIES 
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
        LINK_FLAGS "/INCREMENTAL:NO"
    )
else()
    # GCC/Clang Flags
    target_compile_options(c_error_examples PRIVATE 
        -fsanitize=address 
        -fno-omit-frame-pointer
        -g
        -O0
    )
    target_link_options(c_error_examples PRIVATE -fsanitize=address)
endif()
