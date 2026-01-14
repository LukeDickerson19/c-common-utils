#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sqlite3.h>
#include <stdarg.h>

/*

    compile w/ cmd:
        gcc -std=c11 -Wall -Wextra -Wpedantic sqlite.c -lsqlite3 -o sqlite

        needs the -lsqlite3 arg!

    docs:
        https://www.sqlite.org/c3ref/intro.html
        https://sqlite.org/cintro.html

*/

// Arena allocator (fixed-size, assert on overflow)
typedef struct {
    unsigned char *base;
    size_t cap;
    size_t pos;
} Arena;
static Arena arena_create(void *buffer, size_t size) {
    Arena a = {
        .base = buffer,
        .cap  = size,
        .pos  = 0
    };
    return a;
}
static void arena_reset(Arena *a) {
    a->pos = 0;
}
static void *arena_alloc(Arena *a, size_t size) {
    assert(a->pos + size <= a->cap && "Arena overflow");
    void *ptr = a->base + a->pos;
    a->pos += size;
    return ptr;
}
static char *arena_strdup(Arena *a, const char *s) {
    size_t len = strlen(s) + 1;
    char *p = arena_alloc(a, len);
    memcpy(p, s, len);
    return p;
}

typedef struct {
    char   ***rows;
    size_t   row_count;
    size_t   col_count;
    size_t  *col_widths;
    char   **col_names;
} Table;
static void appendf(
    char *out,
    size_t cap,
    size_t *pos,
    const char *fmt,
    ...
) {
    va_list ap;
    va_start(ap, fmt);

    int n = vsnprintf(out + *pos, cap - *pos, fmt, ap);
    assert(n >= 0);
    assert(*pos + (size_t)n < cap);

    *pos += (size_t)n;
    va_end(ap);
}
static char *pretty_print_table(const Table *t, Arena *arena) {
    if (t->row_count == 0)
        return arena_strdup(arena, "(no rows)\n");

    /* Conservative size estimate */
    size_t est = (t->row_count + 2) * (t->col_count * 32);
    char *out = arena_alloc(arena, est);
    size_t pos = 0;

    /* Header */
    for (size_t i = 0; i < t->col_count; i++)
        appendf(out, est, &pos,
                "%-*s ", (int)t->col_widths[i], t->col_names[i]);
    appendf(out, est, &pos, "\n");

    /* Separator */
    for (size_t i = 0; i < t->col_count; i++) {
        for (size_t j = 0; j < t->col_widths[i]; j++)
            appendf(out, est, &pos, "-");
        appendf(out, est, &pos, " ");
    }
    appendf(out, est, &pos, "\n");

    /* Rows */
    for (size_t r = 0; r < t->row_count; r++) {
        for (size_t c = 0; c < t->col_count; c++)
            appendf(out, est, &pos,
                    "%-*s ", (int)t->col_widths[c], t->rows[r][c]);
        appendf(out, est, &pos, "\n");
    }

    return out;
}

// return SELECT query results using prepared statements to protect against SQL injection
static void load_table(
    sqlite3 *db,
    const char *sql,
    Table *t,
    Arena *arena
) {
    sqlite3_stmt *stmt = NULL;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "prepare failed: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    int col_count = sqlite3_column_count(stmt);

    t->col_count  = col_count;
    t->row_count  = 0;
    t->col_names  = arena_alloc(arena, col_count * sizeof(char *));
    t->col_widths = arena_alloc(arena, col_count * sizeof(size_t));
    t->rows       = NULL;

    for (int i = 0; i < col_count; i++) {
        const char *name = sqlite3_column_name(stmt, i);
        t->col_names[i]  = arena_strdup(arena, name);
        t->col_widths[i] = strlen(name);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char **row = arena_alloc(arena, col_count * sizeof(char *));

        for (int c = 0; c < col_count; c++) {
            const unsigned char *txt = sqlite3_column_text(stmt, c);
            const char *val = txt ? (const char *)txt : "NULL";

            row[c] = arena_strdup(arena, val);

            size_t len = strlen(val);
            if (len > t->col_widths[c])
                t->col_widths[c] = len;
        }

        char ***new_rows =
            arena_alloc(arena, (t->row_count + 1) * sizeof(char **));

        if (t->row_count > 0)
            memcpy(new_rows, t->rows, t->row_count * sizeof(char **));

        new_rows[t->row_count] = row;
        t->rows = new_rows;
        t->row_count++;
    }

    sqlite3_finalize(stmt);
}
static int exec_sql(sqlite3 *db, const char *sql) {
    char *err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
    }
    return rc;
}


int main(void) {
    sqlite3 *db = NULL;
    const char *table = "users";

    unsigned char arena_buf[64 * 1024];
    Arena arena = arena_create(arena_buf, sizeof(arena_buf));

    char * filename = "sqlite_example.db";
    if (sqlite3_open(filename, &db) != SQLITE_OK) {
        fprintf(stderr, "open failed\n");
        return 1;
    } else {
        printf("\nusing sqlite database from file:\n    %s\n", filename);
    }

    char sql[256];
    snprintf(sql, sizeof(sql), "DROP TABLE IF EXISTS %s;", table);
    printf("\nExecuting SQL:\n    %s\n\n", sql);
    exec_sql(db, sql);

    snprintf(sql, sizeof(sql),
             "CREATE TABLE %s ("
             " id INTEGER PRIMARY KEY AUTOINCREMENT,"
             " name TEXT,"
             " age INTEGER);",
             table);
    printf("Executing SQL:\n    %s\n\n", sql);
    exec_sql(db, sql);

    /* --- Insert rows with bound parameters --- */
    sqlite3_stmt *stmt = NULL;
    snprintf(sql, sizeof(sql), "INSERT INTO %s (name, age) VALUES (?, ?);", table);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "prepare failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    struct { const char *name; int age; } users[] = {
        {"Alice", 30}, {"Bob", 24}, {"Carol", 42}, {"Dave", 24}
    };
    for (size_t i = 0; i < sizeof(users)/sizeof(users[0]); i++) {
        sqlite3_bind_text(stmt, 1, users[i].name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, users[i].age);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "insert failed: %s\n", sqlite3_errmsg(db));
            return 1;
        }
        sqlite3_reset(stmt);  // reset for next row
    }
    const char *query_str = sqlite3_expanded_sql(stmt);
    if (query_str) {
        printf("Executing SQL:\n    %s\n\n", query_str);
        sqlite3_free((void*)query_str);
    }
    sqlite3_finalize(stmt);

    /* --- First SELECT --- */
    Table t = {0};
    snprintf(sql, sizeof(sql), "SELECT id, name, age FROM %s;", table);
    printf("Executing SQL:\n    %s\n\n", sql);
    load_table(db, sql, &t, &arena);
    char *out = pretty_print_table(&t, &arena);
    printf("%s\n", out);
    arena_reset(&arena);

    /* --- Delete rows with bound parameter --- */
    const int age_to_delete = 24;
    snprintf(sql, sizeof(sql), "DELETE FROM %s WHERE age = ?;", table);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "prepare failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(stmt, 1, age_to_delete);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "delete failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    query_str = sqlite3_expanded_sql(stmt);
    if (query_str) {
        printf("Executing SQL:\n    %s\n\n", query_str);
        sqlite3_free((void*)query_str);
    }
    sqlite3_finalize(stmt);

    /* --- Second SELECT --- */
    Table t2 = {0};
    snprintf(sql, sizeof(sql), "SELECT id, name, age FROM %s;", table);
    printf("Executing SQL:\n    %s\n\n", sql);
    load_table(db, sql, &t2, &arena);
    out = pretty_print_table(&t2, &arena);
    printf("%s\n", out);

    sqlite3_close(db);
    return 0;
}

