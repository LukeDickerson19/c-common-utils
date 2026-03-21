cmake_minimum_required(VERSION 3.20)

project(c_common_utils
    VERSION 1.0
    DESCRIPTION "Reusable C utility libraries"
    LANGUAGES C
)

# -------------------------------
# Compiler & C standard
# -------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Detect compiler
message(STATUS "Compiler ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")

set(USING_CLANG_WINDOWS OFF)
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND WIN32)
    set(USING_CLANG_WINDOWS ON)
endif()

# -------------------------------
# Default build type
# -------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# -------------------------------
# AddressSanitizer in Debug
# -------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        message(STATUS "AddressSanitizer: ENABLED (Debug mode)")

        # ASan + LeakSanitizer
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g -O0)
        add_link_options(-fsanitize=address)

        # UndefinedBehaviorSanitizer
        add_compile_options(-fsanitize=undefined)
        add_link_options(-fsanitize=undefined)

        # Optional: ThreadSanitizer
        # add_compile_options(-fsanitize=thread)
        # add_link_options(-fsanitize=thread)

        # Environment options for sanitizers
        set(ENV{ASAN_OPTIONS} "detect_leaks=1:abort_on_error=1:symbolize=1")
        set(ENV{UBSAN_OPTIONS} "print_stacktrace=1")
    else()
        message(WARNING "AddressSanitizer only fully supported with Clang")
    endif()
endif()

# -------------------------------
# Strict warnings
# -------------------------------
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2)

# -------------------------------
# Threads
# -------------------------------
find_package(Threads REQUIRED)

# -------------------------------
# Build options
# -------------------------------
option(BUILD_ALL_UTILS "Build all utility libraries" OFF)
option(BUILD_STRING_UTIL "Build string_util library" OFF)
option(BUILD_TIME_UTIL "Build time_util library" OFF)
option(BUILD_LOGGING_UTIL "Build logging_util library" OFF)

if(BUILD_ALL_UTILS)
    set(BUILD_STRING_UTIL ON CACHE BOOL "Build string_util library" FORCE)
    set(BUILD_TIME_UTIL ON CACHE BOOL "Build time_util library" FORCE)
    set(BUILD_LOGGING_UTIL ON CACHE BOOL "Build logging_util library" FORCE)
endif()

# -------------------------------
# Add subdirectories
# -------------------------------
if(BUILD_STRING_UTIL)
    add_subdirectory(string_util)
endif()

if(BUILD_TIME_UTIL)
    add_subdirectory(time_util)
endif()

if(BUILD_LOGGING_UTIL)
    # Ensure dependencies exist
    if(NOT BUILD_STRING_UTIL)
        message(FATAL_ERROR "logging_util requires string_util. Set -DBUILD_STRING_UTIL=ON")
    endif()
    if(NOT BUILD_TIME_UTIL)
        message(FATAL_ERROR "logging_util requires time_util. Set -DBUILD_TIME_UTIL=ON")
    endif()

    add_subdirectory(logging_util)
endif()

