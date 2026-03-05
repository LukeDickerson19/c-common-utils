# if (MSVC)
#     # Windows
#     add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
#     add_compile_options(
#         /W4
#         /wd4996
#         /wd4267
#         /wd4244
#     )
# else()
#     # Linux
#     # add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

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

add_executable(full_example
    tests/full_example.c
)

target_link_libraries(full_example
    PRIVATE time_util
)

