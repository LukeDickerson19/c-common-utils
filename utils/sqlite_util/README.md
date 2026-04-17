# sqlite-util

#### DESCRIPTION

> SQLite utility written in C.

##### Main table struct:
```c
/** SQLite Table struct
 * 
 */
typedef struct {
    char   ***rows;
    size_t   row_count;
    size_t   col_count;
    size_t  *col_widths;
    char   **col_names;
} SQLiteTable;
```

##### Features:

> - convenience functions that wrap the [sqlite3](https://sqlite.org/cintro.html) library
> - Functions:
>   - **Connection**:
>     - sqlite_init_connection()
>     - sqlite_close_connection()
>   - **Query**:
>     - sqlite_execute_query()
>     - sqlite_get_cell_value()
>   - **Print**:
>     - sqlite_pretty_print_table()


#### BUILD & RUN

> See [NOTES.txt](https://github.com/LukeDickerson19/c-common-utils/blob/master/NOTES.txt) section Utils, subsection Usage, for the specific CLI commands to build and run this code.
> Code has been tested on:
>   - Linux (on Manjaro v25.0.10, x86_64) with compilers:
>      - GCC (version 15.2.1)
>      - Clang/LLVM (version 21.1.8)
>   - Windows (on Windows 11, x86_64) with compilers:
>      - MSVC (version 19.50.35723.0)
>      - clang-cl (version 21.1.0)

#### EXAMPLE USAGE
Below is a copy of [tests/sqlite_util_full_example.c](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/sqlite_util/tests/sqlite_util_full_example.c). See [include/sqlite_util.h](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/sqlite_util/include/sqlite_util.h) for all function definitions and descriptions.
```c
```

#### EXAMPLE OUTPUT
```
```

