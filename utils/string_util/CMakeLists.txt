# ---- logging_util library ----

add_library(string_util
    src/string_util.c
)

# Include directories for consumers of logging_util
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