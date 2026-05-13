cmake_minimum_required(VERSION 3.10)
project(sqlite_util)

# sqlite3 is a C library for interfacing with SQLite databases.
# It is built from source for portability and version control.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite3)

# ---- sqlite_util library ----
add_library(sqlite_util
    src/sqlite_util.c
)

# Include directories for sqlite_util and its consumers
target_include_directories(sqlite_util
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite3
)

# Link sqlite3 to sqlite_util
target_link_libraries(sqlite_util PRIVATE sqlite3)

# ---- Tests ----
add_executable(sqlite_util_full_example
    tests/sqlite_util_full_example.c
)
if(WIN32)
    # Windows
    target_link_libraries(sqlite_util_full_example PRIVATE
        sqlite_util
    )
else()
    # Linux
    target_link_libraries(sqlite_util_full_example PRIVATE
        sqlite_util
        m
    )
endif()