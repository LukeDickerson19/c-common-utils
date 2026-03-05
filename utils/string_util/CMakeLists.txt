add_library(string_util
    src/string_util.c
)

target_include_directories(string_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- Tests ----

add_executable(string_util_test
    tests/main.c
)

target_link_libraries(string_util_test
    PRIVATE string_util
)

add_executable(string_util_readme_example
    tests/readme_example.c
)

target_link_libraries(string_util_readme_example
    PRIVATE string_util
)