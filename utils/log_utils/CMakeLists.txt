cmake_minimum_required(VERSION 3.20)
project(log_utils C)

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

target_compile_definitions(log_utils
    PRIVATE _POSIX_C_SOURCE=200809L
)

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

# ---- test executable ----
add_executable(log_utils_test
    tests/main.c
)
target_link_libraries(log_utils_test
    PRIVATE
        log_utils
        cjson
)
# define POSIX macro for Linux when compiling the test executable
if(NOT MSVC)
    target_compile_definitions(log_utils_test
        PRIVATE _POSIX_C_SOURCE=200809L
    )
endif()

enable_testing()
add_test(NAME log_utils_basic_test COMMAND log_utils_test)

# ---- README example executable ----
add_executable(readme_example
    tests/readme_example.c
)
target_link_libraries(readme_example
    PRIVATE
        log_utils
        cjson
)
