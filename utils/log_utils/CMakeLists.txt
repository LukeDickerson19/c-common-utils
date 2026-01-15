cmake_minimum_required(VERSION 3.20)
project(log_utils C)

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

# ---- log_utils library ----
add_library(log_utils
    src/log_utils.c
)

if(NOT MSVC)
    target_compile_definitions(log_utils
        PRIVATE _POSIX_C_SOURCE=200809L
    )
endif()

target_include_directories(log_utils
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

target_link_libraries(log_utils
    PUBLIC Threads::Threads
)

# ---- test executables ----
add_executable(log_utils_test tests/main.c)
add_executable(readme_example tests/readme_example.c)
add_executable(test_thread_safety tests/test_thread_safety.c)

target_link_libraries(log_utils_test PRIVATE log_utils cjson)
target_link_libraries(readme_example PRIVATE log_utils cjson)
target_link_libraries(test_thread_safety PRIVATE log_utils)

if(NOT MSVC)
    target_compile_definitions(log_utils_test
        PRIVATE _POSIX_C_SOURCE=200809L
    )
endif()

