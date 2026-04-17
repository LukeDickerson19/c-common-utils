cmake_minimum_required(VERSION 3.20)

# ---------------------------------------------
# Compiler   Platform    C standard  VLA support
#
# gcc        Linux       C99         Yes (C99 VLAs allowed)
# clang      Linux       C99         Yes (C99 VLAs allowed)
# msvc       Windows     C99         No  (MSVC never supports VLAs)
# clang-cl   Windows     C99/C11     Yes (VLAs supported via Clang backend)
# ---------------------------------------------

# ---------------------------------------------
# User must pass -DCOMPILER=<compiler>
# Valid options: gcc, clang, msvc, clang-cl
# ---------------------------------------------
if(NOT DEFINED COMPILER)
    message(FATAL_ERROR "COMPILER is required. Valid options are:\n
    gcc        - GCC on Linux, C99\n
    clang      - Clang on Linux, C99, VLAs allowed\n
    msvc       - MSVC on Windows, C99, VLAs NOT allowed\n
    clang-cl   - Clang-cl on Windows, C99, VLAs allowed if /std:c11")
endif()

string(TOLOWER "${COMPILER}" COMPILER)

# ---------------------------------------------
# Platform validation
# ---------------------------------------------
if((${COMPILER} STREQUAL "gcc" OR ${COMPILER} STREQUAL "clang") AND NOT UNIX)
    message(FATAL_ERROR "${COMPILER} is only valid on Linux/macOS")
endif()

if((${COMPILER} STREQUAL "msvc" OR ${COMPILER} STREQUAL "clang-cl") AND NOT WIN32)
    message(FATAL_ERROR "${COMPILER} is only valid on Windows")
endif()

# ---------------------------------------------
# Set compilers BEFORE project()
# ---------------------------------------------
if(${COMPILER} STREQUAL "gcc")
    set(CMAKE_C_COMPILER "gcc" CACHE FILEPATH "C compiler" FORCE)
elseif(${COMPILER} STREQUAL "clang")
    set(CMAKE_C_COMPILER "clang" CACHE FILEPATH "C compiler" FORCE)
elseif(${COMPILER} STREQUAL "msvc")
    # MSVC usually detected automatically
elseif(${COMPILER} STREQUAL "clang-cl")
    if(WIN32)
        if(NOT DEFINED CLANG_CL_PATH OR NOT EXISTS "${CLANG_CL_PATH}")
            message(FATAL_ERROR "CLANG_CL_PATH must be defined and point to a valid clang-cl executable")
        endif()
        # Use clang-cl for both C and C++
        set(CMAKE_C_COMPILER "${CLANG_CL_PATH}" CACHE FILEPATH "C compiler" FORCE)
        set(CMAKE_CXX_COMPILER "${CLANG_CL_PATH}" CACHE FILEPATH "C++ compiler" FORCE)

        # Enable /std:c11 for VLAs in C, pick reasonable C++ standard
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c11")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    else()
        message(FATAL_ERROR "clang-cl is only supported on Windows")
    endif()
endif()

# ------------------------------------------------
# Set C and C++ compilers on Windows if using clang-cl (propagates to subprojects)
# ------------------------------------------------
if(WIN32 AND DEFINED CLANG_CL_PATH)
    set(CMAKE_C_COMPILER   "${CLANG_CL_PATH}" CACHE FILEPATH "Path to clang-cl" FORCE)
    set(CMAKE_CXX_COMPILER "${CLANG_CL_PATH}" CACHE FILEPATH "Path to clang-cl" FORCE)
endif()

# ------------------------------------------------
# Set C standard before project() so it propagates to subprojects
# ------------------------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ------------------------------------------------
# Project declaration 
# ------------------------------------------------
project(c_common_utils
    VERSION 1.0
    DESCRIPTION "Reusable C utility libraries"
    LANGUAGES C CXX
)

message(STATUS "Selected compiler: ${COMPILER}")
message(STATUS "C standard enforced: C${CMAKE_C_STANDARD}")
message(STATUS "C++ standard enforced: C++${CMAKE_CXX_STANDARD}")

# ---------------------------------------------
# Default build type
# ---------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ---------------------------------------------
# AddressSanitizer in Debug
# ---------------------------------------------
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_ASAN)
    if(${COMPILER} STREQUAL "clang" OR ${COMPILER} STREQUAL "clang-cl" OR ${COMPILER} STREQUAL "gcc")
        message(STATUS "AddressSanitizer: ENABLED")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g -O0)
        add_link_options(-fsanitize=address)
    else()
        message(WARNING "AddressSanitizer not supported on MSVC")
    endif()
else()
    message(STATUS "AddressSanitizer: DISABLED")
endif()

# ---------------------------------------------
# Strict warnings
# ---------------------------------------------
if(NOT MSVC)
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2)
endif()

# ---------------------------------------------
# Threads
# ---------------------------------------------
find_package(Threads REQUIRED)

# ---------------------------------------------
# Build options
# ---------------------------------------------
option(BUILD_ALL_UTILS "Build all utility libraries" OFF)
option(BUILD_STRING_UTIL "Build string_util library" OFF)
option(BUILD_TIME_UTIL "Build time_util library" OFF)
option(BUILD_LOGGING_UTIL "Build logging_util library" OFF)
option(BUILD_SQLITE_UTIL "Build sqlite_util library" OFF)

if(BUILD_ALL_UTILS)
    set(BUILD_STRING_UTIL ON CACHE BOOL "Build string_util library" FORCE)
    set(BUILD_TIME_UTIL ON CACHE BOOL "Build time_util library" FORCE)
    set(BUILD_LOGGING_UTIL ON CACHE BOOL "Build logging_util library" FORCE)
    set(BUILD_SQLITE_UTIL ON CACHE BOOL "Build sqlite_util library" FORCE)
endif()

# ---------------------------------------------
# Add subdirectories
# ---------------------------------------------
if(BUILD_STRING_UTIL)
    add_subdirectory(string_util)
endif()

if(BUILD_TIME_UTIL)
    add_subdirectory(time_util)
endif()

if(BUILD_LOGGING_UTIL)
    if(NOT BUILD_STRING_UTIL)
        message(FATAL_ERROR "logging_util requires string_util. Set -DBUILD_STRING_UTIL=ON")
    endif()
    if(NOT BUILD_TIME_UTIL)
        message(FATAL_ERROR "logging_util requires time_util. Set -DBUILD_TIME_UTIL=ON")
    endif()
    add_subdirectory(logging_util)
endif()

if(BUILD_SQLITE_UTIL)
    add_subdirectory(sqlite_util)
endif()

