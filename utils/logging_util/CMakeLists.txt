# ---- logging_util library ----

add_library(logging_util
    src/logging_util.c
)

target_include_directories(logging_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(logging_util
    PUBLIC
        string_util
        time_util
        Threads::Threads
)

if(NOT MSVC)
    target_compile_definitions(logging_util PRIVATE _POSIX_C_SOURCE=200809L)
endif()

# ---- cJSON (vendored for tests) ----

add_library(cjson STATIC
    external/cjson/cJSON.c
)

target_include_directories(cjson
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/cjson
)

# ---- Tests ----

add_executable(logging_util_full_example
    tests/full_example.c
)

target_link_libraries(logging_util_full_example
    PRIVATE logging_util cjson
)

add_executable(logging_util_readme_example
    tests/readme_example.c
)

target_link_libraries(logging_util_readme_example
    PRIVATE logging_util cjson
)

add_executable(logging_util_thread_test
    tests/test_thread_safety.c
)

target_link_libraries(logging_util_thread_test
    PRIVATE logging_util
)

if(NOT MSVC)
    target_compile_definitions(logging_util_full_example PRIVATE _POSIX_C_SOURCE=200809L)
endif()