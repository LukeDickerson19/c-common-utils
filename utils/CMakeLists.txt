cmake_minimum_required(VERSION 3.20)

project(c_common_utils
    VERSION 1.0
    DESCRIPTION "Reusable C utility libraries"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build ALL using AddressSanitizer:
#     cmake -S . -B build -DBUILD_ALL_UTILS=ON -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug # Configure with ASan and Debug symbols
#     cmake --build build
# Run with GDB (logging_util example):
#     export ASAN_OPTIONS=abort_on_error=1
#     gdb ./build/logging_util/logging_util_full_example
#     Inside GDB:
#         (gdb) run
#         ... wait for crash ...
#         (gdb) bt
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    if(MSVC)
        set(ASAN_FLAGS /fsanitize=address /Zi /Od)
        set(ASAN_LINK_FLAGS /fsanitize=address /INCREMENTAL:NO)
    else()
        # Adding -fno-omit-frame-pointer and -g is vital for GDB traces
        set(ASAN_FLAGS -fsanitize=address -fno-omit-frame-pointer -g -O0)
        set(ASAN_LINK_FLAGS -fsanitize=address)
    endif()
    
    # Apply to all libraries/executables defined globally
    add_compile_options(${ASAN_FLAGS})
    add_link_options(${ASAN_LINK_FLAGS})
endif()

# Option to build everything at once
option(BUILD_ALL_UTILS "Build all utility libraries" OFF)

# Individual utility build options
option(BUILD_STRING_UTIL "Build string_util library" OFF)
option(BUILD_TIME_UTIL "Build time_util library" OFF)
option(BUILD_LOGGING_UTIL "Build logging_util library" OFF)

# If BUILD_ALL_UTILS is ON, toggle all individual options ON
if(BUILD_ALL_UTILS)
    set(BUILD_STRING_UTIL ON CACHE BOOL "Build string_util library" FORCE)
    set(BUILD_TIME_UTIL ON CACHE BOOL "Build time_util library" FORCE)
    set(BUILD_LOGGING_UTIL ON CACHE BOOL "Build logging_util library" FORCE)
endif()

# Build ALL utils:
#    cd c-common-utils/utils
#    cmake -S . -B build -DBUILD_ALL_UTILS=ON
#    cmake --build build

# Build ONLY string_util:
#    cd c-common-utils/utils
#    cmake -S . -B build -DBUILD_STRING_UTIL=ON
#    cmake --build build
#    ./build/string_util/string_util_readme_example
#    ./build/string_util/string_util_full_example
if(BUILD_STRING_UTIL)
    add_subdirectory(string_util)
endif()

# Build ONLY time_util:
#    cd c-common-utils/utils
#     cmake -S . -B build -DBUILD_TIME_UTIL=ON
#     cmake --build build
#     ./build/time_util/time_util_full_example
if(BUILD_TIME_UTIL)
    add_subdirectory(time_util)
endif()

# Build ONLY logging_util:
#    cd c-common-utils/utils
#    cmake -S . -B build -DBUILD_LOGGING_UTIL=ON -DBUILD_STRING_UTIL=ON -DBUILD_TIME_UTIL=ON
#    cmake --build build
#    ./build/logging_util/logging_util_readme_example
#    ./build/logging_util/logging_util_full_example
if(BUILD_LOGGING_UTIL)
    find_package(Threads REQUIRED)

    # Ensure dependencies exist
    if(NOT BUILD_STRING_UTIL)
        message(FATAL_ERROR "logging_util requires string_util. Set -DBUILD_STRING_UTIL=ON")
    endif()
    if(NOT BUILD_TIME_UTIL)
        message(FATAL_ERROR "logging_util requires time_util. Set -DBUILD_TIME_UTIL=ON")
    endif()

    add_subdirectory(logging_util)
endif()
