#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

/*

    build with cmd:
        gcc -std=c11 -Wall -Wextra -Wpedantic -O2 table_struct.c -o table_struct
            flags explaination:
                -std=c11     Uses modern C (needed for bool, static inline, etc.)
                -Wall        Enables common warnings
                -Wextra      Enables more useful warnings
                -Wpedantic   Enforces strict ISO C compliance (you’ve been catching real bugs with this)
                -O2          Reasonable optimization level (safe, no UB exploitation)

*/

/* ---------- utilities ---------- */

static char *c_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *p = malloc(len);
    assert(p);
    memcpy(p, s, len);
    return p;
}

/* ---------- data model ---------- */

typedef struct {
    int     col_int;
    float   col_float;
    bool    col_bool;
    char   *col_string;
    time_t  col_datetime;
} Row;

typedef struct {
    Row   *rows;
    size_t count;
    size_t capacity;
} Table;

/* ---------- table ops ---------- */

static void table_init(Table *t) {
    t->rows = NULL;
    t->count = 0;
    t->capacity = 0;
}

static void table_grow(Table *t) {
    size_t new_cap = t->capacity ? t->capacity * 2 : 4;
    Row *new_rows = realloc(t->rows, new_cap * sizeof(Row));
    assert(new_rows);
    t->rows = new_rows;
    t->capacity = new_cap;
}

static void table_add_row(
    Table *t,
    int col_int,
    float col_float,
    bool col_bool,
    const char *col_string,
    time_t col_datetime
) {
    if (t->count == t->capacity)
        table_grow(t);

    Row *r = &t->rows[t->count++];
    r->col_int      = col_int;
    r->col_float    = col_float;
    r->col_bool     = col_bool;
    r->col_string   = c_strdup(col_string);
    r->col_datetime = col_datetime;
}

static void table_remove_row(Table *t, size_t index) {
    assert(index < t->count);

    free(t->rows[index].col_string);

    for (size_t i = index + 1; i < t->count; i++)
        t->rows[i - 1] = t->rows[i];

    t->count--;
}

static void table_free(Table *t) {
    for (size_t i = 0; i < t->count; i++)
        free(t->rows[i].col_string);

    free(t->rows);
    t->rows = NULL;
    t->count = 0;
    t->capacity = 0;
}

/* ---------- pretty print ---------- */

static char *pretty_print_table(const Table *t) {
    if (t->count == 0)
        return c_strdup("(empty table)\n");

    /* conservative size estimate */
    size_t cap = 256 + t->count * 128;
    char *out = malloc(cap);
    assert(out);

    size_t pos = 0;

    pos += snprintf(out + pos, cap - pos,
        "%-6s %-8s %-6s %-10s %-20s\n",
        "INT", "FLOAT", "BOOL", "STRING", "DATETIME");

    pos += snprintf(out + pos, cap - pos,
        "-------------------------------------------------------------\n");

    for (size_t i = 0; i < t->count; i++) {
        char timebuf[32];
        struct tm *tm = localtime(&t->rows[i].col_datetime);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm);

        pos += snprintf(out + pos, cap - pos,
            "%-6d %-8.2f %-6s %-10s %-20s\n",
            t->rows[i].col_int,
            t->rows[i].col_float,
            t->rows[i].col_bool ? "true" : "false",
            t->rows[i].col_string,
            timebuf);
    }

    return out;
}

/* ---------- demo ---------- */

int main(void) {
    Table t;
    table_init(&t);
    printf("\nCreated empty table t\n");

    time_t now = time(NULL);

    table_add_row(&t, 1, 3.14f, true,  "Alice", now);
    table_add_row(&t, 2, 2.71f, false, "Bob",   now);
    table_add_row(&t, 3, 1.41f, true,  "Carol", now);

    char *s = pretty_print_table(&t);
    printf("\nAdded some rows:\n%s\n", s);
    free(s);

    table_remove_row(&t, 1);  /* remove Bob */

    s = pretty_print_table(&t);
    printf("Removed a row:\n%s\n", s);
    free(s);

    table_free(&t);
    printf("Deleted table t\n\n");
    return 0;
}
