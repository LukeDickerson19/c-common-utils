cmake_minimum_required(VERSION 3.10)
project(string_util)

# ---- Find utf8proc ----
# Use vendored copy in external/utf8proc for portability across platforms
set(UTF8PROC_BASE "${CMAKE_CURRENT_SOURCE_DIR}/external/utf8proc")
if(WIN32)
    set(UTF8PROC_DIR "${UTF8PROC_BASE}/windows")
    set(UTF8PROC_LIB "${UTF8PROC_DIR}/lib/utf8proc_static.lib")
else()
    set(UTF8PROC_DIR "${UTF8PROC_BASE}/linux")
    set(UTF8PROC_LIB "${UTF8PROC_DIR}/lib/libutf8proc.a")
endif()

add_library(utf8proc::utf8proc STATIC IMPORTED)
set_target_properties(utf8proc::utf8proc PROPERTIES
    IMPORTED_LOCATION             "${UTF8PROC_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${UTF8PROC_DIR}/include"
    INTERFACE_COMPILE_DEFINITIONS "UTF8PROC_STATIC"
)


# ---- string_util library ----
add_library(string_util
    src/string_util.c
)

# Include directories for string_util and its consumers
target_include_directories(string_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${UTF8PROC_DIR}/include
)

# Link utf8proc to string_util
target_link_libraries(string_util
    PRIVATE utf8proc::utf8proc
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
