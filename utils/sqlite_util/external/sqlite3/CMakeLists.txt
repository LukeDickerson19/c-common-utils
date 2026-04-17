cmake_minimum_required(VERSION 3.10)

project(sqlite3 C)

add_library(sqlite3 STATIC
    sqlite3.c
)

target_include_directories(sqlite3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(sqlite3 PUBLIC
    SQLITE_THREADSAFE=1              # Serialized threading mode (safe default)
    SQLITE_OMIT_LOAD_EXTENSION       # Disable runtime extension loading (security)
    SQLITE_ENABLE_COLUMN_METADATA    # Access table/column origin info
    SQLITE_ENABLE_FTS5               # Enable full-text search
    SQLITE_OMIT_DEPRECATED           # Remove legacy APIs
    SQLITE_OMIT_PROGRESS_CALLBACK    # Remove rarely used interrupt callback
    SQLITE_DEFAULT_FOREIGN_KEYS=1    # Enforce foreign keys by default
    SQLITE_TEMP_STORE=2              # Use memory for temp tables (faster)
    SQLITE_ENABLE_EXPLAIN_COMMENTS   # Better EXPLAIN output (debugging)
)

