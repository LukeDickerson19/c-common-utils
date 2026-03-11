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
# cmake -S . -B build # see below for optional args
# cmake --build build
#
# Args:
#     Release Mode (ENABLE_ASAN is OFF):
#         cmake -S . -B build -DBUILD_ALL_UTILS=ON 
#         cmake -S . -B build -DBUILD_ALL_UTILS=ON -DCMAKE_BUILD_TYPE=Release
#    
#     Debug Mode (ENABLE_ASAN is ON):
#         cmake -S . -B build -DBUILD_ALL_UTILS=ON  -DCMAKE_BUILD_TYPE=Debug
#    
#     Debug Mode (ENABLE_ASAN is OFF, useful for GDB session without the ASan overhead):
#         cmake -S . -B build -DBUILD_ALL_UTILS=ON -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=OFF

# Run with GDB + gdb script (logging_util example):
#     ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:strict_string_checks=1:detect_stack_use_after_return=1:detect_stack_use_after_scope=1:symbolize=1 UBSAN_OPTIONS=print_stacktrace=1 gdb ./build/logging_util/logging_util_full_example -x logging_util/scripts/debugging_scratchpad.gdb

# Set Default Build Type to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
endif()

# Auto-enable ASan for Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT DEFINED ENABLE_ASAN)
    set(ENABLE_ASAN ON CACHE BOOL "Enable AddressSanitizer")
else()
    option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
endif()

# Apply Flags based on the final ENABLE_ASAN state
if(ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address /Zi /Od)
        add_link_options(/fsanitize=address /INCREMENTAL:NO)
    else()
        # Common ASan flags
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
        
        # Add Debug-specific performance/symbol flags
        add_compile_options($<$<CONFIG:Debug>:-g> $<$<CONFIG:Debug>:-O0>)
    endif()
    message(STATUS "AddressSanitizer: ENABLED")
else()
    message(STATUS "AddressSanitizer: DISABLED")
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
