cmake_minimum_required(VERSION 3.20)
project(ascii_string_util
    VERSION 1.0
    DESCRIPTION "older version of string_util that didn't support UTF-8, only ASCII"
    LANGUAGES C
)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---- string_util library ----

add_library(string_util
    src/string_util.c
)

# Include directories for consumers of string_util
target_include_directories(string_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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