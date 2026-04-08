cmake_minimum_required(VERSION 3.10)
project(string_util)

# utf8proc is a C library for processing UTF-8 encoded Unicode strings.
# It is built from source for portability and version control.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/utf8proc)

# ---- string_util library ----
add_library(string_util
    src/string_util.c
)

# Include directories for string_util and its consumers
target_include_directories(string_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/utf8proc
)

# Link utf8proc to string_util
target_link_libraries(string_util PRIVATE utf8proc)

# ---- Tests ----
add_executable(string_util_full_example
    tests/string_util_full_example.c
)
target_link_libraries(string_util_full_example
    PRIVATE string_util
)

add_executable(string_util_readme_example
    tests/string_util_readme_example.c
)
target_link_libraries(string_util_readme_example
    PRIVATE string_util
)
