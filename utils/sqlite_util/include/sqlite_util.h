#ifndef SQLITE_UTILS_H
#define SQLITE_UTILS_H

#include <stddef.h>
#include <sqlite3.h>

// ======================== Table Struct ========================

typedef struct {
    char    ***rows;
    size_t    row_count;
    size_t    col_count;
    size_t   *col_widths;
    char    **col_names;
    // private — do not touch directly
    void     *_arena_buf;
    size_t    _arena_cap;
} SQLiteTable;

void sqlite_table_free(
    SQLiteTable *t
);

// ======================== Pretty Print Options ========================

typedef struct {
    size_t max_columns;      // 0 = no limit
    size_t max_column_width; // 0 = no limit
    size_t max_rows;         // 0 = no limit
} SQLitePrettyPrintOptions;

// ======================== Query ========================

// Run any SQL statement against db.
// If t is non-NULL and the query returns columns, results are loaded into t.
// Caller must call sqlite_table_free(t) when done.
// If t is NULL, or the query produces no columns (INSERT, CREATE, etc.),
// runs fire-and-forget. Returns a SQLITE_* result code.
int sqlite_execute(
    sqlite3 *db,
    const char *query,
    SQLiteTable *t
);

int sqlite_insert(
    sqlite3      *db,
    const char   *table,
    const void  **values,
    size_t        value_count
);

// ======================== Print ========================

// Returns a pointer into a caller-supplied buffer — no extra allocation.
// Pass NULL for opt to use defaults.
void sqlite_pretty_print(
    const SQLiteTable *t,
    const SQLitePrettyPrintOptions *opt,
    char *out,
    size_t out_cap
);

// ======================= Get Cell's Value =====================

// Returns the value at (row_index, column_name) as a string, or NULL if not found.
const char *sqlite_table_get(
    const SQLiteTable *t,
    size_t row_index,
    const char *column_name
);

#endif /* SQLITE_UTILS_H */

