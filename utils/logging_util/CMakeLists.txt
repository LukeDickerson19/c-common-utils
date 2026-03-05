# ---- logging_util library ----

add_library(logging_util
    src/logging_util.c
)

# Include directories for consumers of logging_util
target_include_directories(logging_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(logging_util
    PUBLIC
        string_util
        time_util
        Threads::Threads
)

# POSIX feature macros for Linux/macOS
if(NOT MSVC)
    target_compile_definitions(logging_util PRIVATE _POSIX_C_SOURCE=200809L)
endif()


# ---- cJSON (vendored, only for tests) ----
add_library(cjson STATIC
    external/cjson/cJSON.c
)

target_include_directories(cjson
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/cjson
)


# ---- Test executables ----

# Helper function to add a test executable
function(add_logging_util_test TEST_NAME SRC_FILE)
    add_executable(${TEST_NAME} tests/${SRC_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE logging_util cjson)
    if(NOT MSVC)
        target_compile_definitions(${TEST_NAME} PRIVATE _POSIX_C_SOURCE=200809L)
    endif()
endfunction()

add_logging_util_test(logging_util_full_example logging_util_full_example.c)
add_logging_util_test(logging_util_readme_example logging_util_readme_example.c)
add_logging_util_test(logging_util_thread_test test_thread_safety.c)