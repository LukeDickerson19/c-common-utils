cmake_minimum_required(VERSION 3.20)
project(string_util C)

find_package(Threads REQUIRED)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---- Compiler Warnings ----
if (MSVC)
    # Windows
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options(
        /W4
        /wd4996
        /wd4267
        /wd4244
    )
else()
    # Linux
    # add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---- string_util library ----
add_library(string_util
    src/string_util.c
)

if(NOT MSVC)
    target_compile_definitions(string_util
        PRIVATE _POSIX_C_SOURCE=200809L
    )
endif()

target_include_directories(string_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- test executables ----
add_executable(string_util_test tests/main.c)
add_executable(readme_example tests/readme_example.c)

target_link_libraries(string_util_test PRIVATE string_util)
target_link_libraries(readme_example PRIVATE string_util)

if(NOT MSVC)
    target_compile_definitions(string_util_test
        PRIVATE _POSIX_C_SOURCE=200809L
    )
endif()

