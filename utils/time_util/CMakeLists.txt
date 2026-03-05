add_library(time_util
    src/time_util.c
)

if(NOT MSVC)
    target_compile_definitions(time_util
        PRIVATE _POSIX_C_SOURCE=200809L
    )
endif()

target_include_directories(time_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- Tests ----

add_executable(time_util_full_example
    tests/time_util_full_example.c
)

target_link_libraries(time_util_full_example
    PRIVATE time_util
)

