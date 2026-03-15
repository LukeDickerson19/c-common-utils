cmake_minimum_required(VERSION 3.20)
project(logging_util C)

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

# ---- logging_util library ----
add_library(logging_util
    src/logging_util.c
)

if(NOT MSVC)
    target_compile_definitions(logging_util
        PRIVATE _POSIX_C_SOURCE=200809L
    )
endif()

target_include_directories(logging_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- cJSON (vendored, only for tests) ----
add_library(cjson STATIC
    external/cjson/cJSON.c
)

target_include_directories(cjson
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/cjson
)

target_link_libraries(logging_util
    PUBLIC Threads::Threads
)

# ---- test executables ----
add_executable(logging_util_full_example tests/logging_util_full_example.c)
add_executable(logging_util_readme_example tests/logging_util_readme_example.c)

target_link_libraries(logging_util_full_example PRIVATE logging_util cjson)
target_link_libraries(logging_util_readme_example PRIVATE logging_util cjson)

if(NOT MSVC)
    target_compile_definitions(logging_util_full_example
        PRIVATE _POSIX_C_SOURCE=200809L
    )
    target_compile_definitions(logging_util_readme_example
        PRIVATE _POSIX_C_SOURCE=200809L
    )
endif()

