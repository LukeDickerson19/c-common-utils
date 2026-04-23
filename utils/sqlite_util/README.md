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

> - convenience functions that wrap the [sqlite3](https://sqlite.org/cintro.html) library using [sqlite3.c](https://github.com/clibs/sqlite/blob/master/sqlite3.c) lib
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
See [tests/sqlite_util_full_example.c](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/sqlite_util/tests/sqlite_util_full_example.c). See [include/sqlite_util.h](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/sqlite_util/include/sqlite_util.h) for all function definitions and descriptions.

#### EXAMPLE OUTPUT
```
[luke@luke utils]$ ./build/sqlite_util/sqlite_util_full_example 

Using sqlite database:
    sqlite_example.db

Executing SQL:
    DROP TABLE IF EXISTS users;

Executing SQL:
    CREATE TABLE users ( id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, age INTEGER, city TEXT);

Executing SQL:
    SELECT id, name, age, city FROM users;

Full table (UTF-8 names and cities):
id name  age city      
-- ----- --- --------- 
1  Ãlice 30  São Paulo 
2  Björn 24  Zürich    
3  Søren 42  Tōkyō     
4  日本語   24  Réykjavík 
5  Ünité 35  London    
6  Dave  28  Mos̈cow   

With max_column_width = 5 (truncation must not split UTF-8 sequences):
id name  age city  
-- ----- --- ----- 
1  Ãlice 30  Sã... 
2  Björn 24  Zü... 
3  Søren 42  Tōkyō 
4  日本語   24  Ré... 
5  Ünité 35  Lo... 
6  Dave  28  Mo... 

Executing SQL:
    DELETE FROM users WHERE age = 24;

Executing SQL:
    SELECT id, name, age, city FROM users;

After DELETE WHERE age = 24:
id name  age city      
-- ----- --- --------- 
1  Ãlice 30  São Paulo 
3  Søren 42  Tōkyō     
5  Ünité 35  London    
6  Dave  28  Mos̈cow   

Value at row 0, column 'name': Ãlice
Value at row 1, column 'city': Tōkyō
Value at row 1, column 'age': 42
[luke@luke utils]$ 
[luke@luke utils]$ 
```

