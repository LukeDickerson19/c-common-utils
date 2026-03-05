cmake_minimum_required(VERSION 3.20)

project(c_common_utils
    VERSION 1.0
    DESCRIPTION "Reusable C utility libraries"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Options to enable/disable individual utilities builds with ON/OFF to test utilities individually
# Example: Only build time_util:
#     cmake -S . -B build -DBUILD_TIME_UTIL=ON
#     cmake --build build
#     ./build/time_util/full_example
option(BUILD_STRING_UTIL "Build string_util library" OFF)
option(BUILD_TIME_UTIL "Build time_util library" OFF)
option(BUILD_LOGGING_UTIL "Build logging_util library" OFF)

# Conditional adds
if(BUILD_STRING_UTIL)
    add_subdirectory(string_util)
endif()

if(BUILD_TIME_UTIL)
    add_subdirectory(time_util)
endif()

if(BUILD_LOGGING_UTIL)
    add_subdirectory(logging_util)
endif()