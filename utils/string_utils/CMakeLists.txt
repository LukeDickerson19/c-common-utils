cmake_minimum_required(VERSION 3.20)

project(string_utils C)

# Use a modern C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Warnings (good defaults)
add_compile_options(-Wall -Wextra -Wpedantic)

# ---- Library ----
add_library(string_utils
    src/string_utils.c
)

target_include_directories(string_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- Test executable ----
add_executable(string_utils_test
    tests/main.c
)

target_link_libraries(string_utils_test
    PRIVATE string_utils
)

# ---- Optional: make `make test` work later ----
enable_testing()
add_test(NAME string_utils_basic_test COMMAND string_utils_test)
