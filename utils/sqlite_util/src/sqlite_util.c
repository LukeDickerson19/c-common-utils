#include "sqlite_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

// ======================== Arena (internal) ========================

typedef struct {
    void  *base;
    size_t cap;
    size_t pos;
} Arena;

static Arena arena_create(void *buffer, size_t size) {
    Arena a = { .base = buffer, .cap = size, .pos = 0 };
    return a;
}

static void *arena_alloc(
    Arena *a,
    size_t size
) {
    assert(a->pos + size <= a->cap && "Arena overflow");
    void *ptr = (char *)a->base + a->pos;
    a->pos += size;
    return ptr;
}

static char *arena_strdup(
    Arena *a,
    const char *s
) {
    size_t len = strlen(s) + 1;
    char *p = arena_alloc(a, len);
    memcpy(p, s, len);
    return p;
}

// ======================== Internal Helpers ========================

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
    assert(n >= 0 && (size_t)n < cap - *pos);
    *pos += (size_t)n;
    va_end(ap);
}

static size_t display_len(
    const char *s
) {
    if (!s) return 4; // "NULL"
    size_t len = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; ) {
        if      (*p < 0x80)           { p += 1; len += 1; }
        else if ((*p & 0xE0) == 0xC0) { p += 2; len += 1; }
        else if ((*p & 0xF0) == 0xE0) { p += 3; len += 2; }
        else if ((*p & 0xF8) == 0xF0) { p += 4; len += 2; }
        else                          { p += 1; len += 1; }
    }
    return len;
}

static size_t effective_width(
    size_t c,
    const SQLiteTable *t,
    const SQLitePrettyPrintOptions *opt
) {
    size_t w = t->col_widths[c];
    if (opt->max_column_width && opt->max_column_width < w)
        w = opt->max_column_width;
    return w;
}

static void write_cell_padded(
    char *out,
    size_t cap,
    size_t *pos,
    const char *s,
    size_t width
) {
    if (!s) s = "NULL";
    size_t len = display_len(s);

    if (len > width) {
        if (width <= 3) { appendf(out, cap, pos, "..."); return; }
        size_t copy_len = width - 3;
        char tmp[256];
        if (copy_len > sizeof(tmp) - 1) copy_len = sizeof(tmp) - 1;
        memcpy(tmp, s, copy_len);
        tmp[copy_len] = '\0';
        appendf(out, cap, pos, "%s...", tmp);
        return;
    }

    appendf(out, cap, pos, "%s", s);
    for (size_t i = len; i < width; i++)
        appendf(out, cap, pos, " ");
}

// ======================== Table ========================

void sqlite_table_free(SQLiteTable *t) {
    if (!t || !t->_arena_buf) return;
    free(t->_arena_buf);
    memset(t, 0, sizeof(*t));
}

// ======================== Query ========================

// Initial arena size — resized if needed via realloc before populating.
// For most queries this is plenty; tune if you expect very large result sets.
#define INITIAL_ARENA_SIZE (1024 * 1024) // 1 MB

int sqlite_execute(
    sqlite3 *db,
    const char *query,
    SQLiteTable *t
) {
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite_query prepare: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    int col_count = sqlite3_column_count(stmt);

    // Fire-and-forget path: no table requested, or query returns no columns
    if (!t || col_count == 0) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE || rc == SQLITE_ROW) rc = SQLITE_OK;
        else fprintf(stderr, "sqlite_query step: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    }

    // Load path: allocate arena and populate table
    void *buf = malloc(INITIAL_ARENA_SIZE);
    if (!buf) { sqlite3_finalize(stmt); return SQLITE_NOMEM; }

    Arena a = arena_create(buf, INITIAL_ARENA_SIZE);

    t->col_count  = (size_t)col_count;
    t->row_count  = 0;
    t->rows       = NULL;
    t->_arena_buf = buf;
    t->_arena_cap = INITIAL_ARENA_SIZE;

    t->col_names  = arena_alloc(&a, col_count * sizeof(char *));
    t->col_widths = arena_alloc(&a, col_count * sizeof(size_t));

    for (int i = 0; i < col_count; i++) {
        const char *name = sqlite3_column_name(stmt, i);
        t->col_names[i]  = arena_strdup(&a, name);
        t->col_widths[i] = strlen(name);
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        char **row = arena_alloc(&a, col_count * sizeof(char *));

        for (int c = 0; c < col_count; c++) {
            const unsigned char *txt = sqlite3_column_text(stmt, c);
            const char *val = txt ? (const char *)txt : "NULL";
            row[c] = arena_strdup(&a, val);
            size_t len = strlen(val);
            if (len > t->col_widths[c]) t->col_widths[c] = len;
        }

        char ***new_rows = arena_alloc(&a, (t->row_count + 1) * sizeof(char **));
        if (t->row_count > 0)
            memcpy(new_rows, t->rows, t->row_count * sizeof(char **));
        new_rows[t->row_count] = row;
        t->rows = new_rows;
        t->row_count++;
    }

    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) return SQLITE_OK;

    fprintf(stderr, "sqlite_query step: %s\n", sqlite3_errmsg(db));
    sqlite_table_free(t);
    return rc;
}

// ======================== Insert ========================

#define MAX_COLS 64

typedef enum { PRAGMA_INT, PRAGMA_REAL, PRAGMA_TEXT, PRAGMA_BLOB } PragmaType;

static PragmaType parse_pragma_type(const char *s) {
    if (!s) return PRAGMA_TEXT;
    // SQLite type affinity rules — just cover the common cases
    if (strstr(s, "INT"))                           return PRAGMA_INT;
    if (strstr(s, "REAL") || strstr(s, "FLOAT")
                           || strstr(s, "DOUBLE"))  return PRAGMA_REAL;
    if (strstr(s, "BLOB"))                          return PRAGMA_BLOB;
    return PRAGMA_TEXT; // TEXT, VARCHAR, etc.
}

int sqlite_insert(
    sqlite3      *db,
    const char   *table,
    const void  **values,
    size_t        value_count
) {
    // --- 1. Load column info from PRAGMA ---
    char pragma_sql[256];
    snprintf(pragma_sql, sizeof(pragma_sql), "PRAGMA table_info(%s);", table);

    sqlite3_stmt *pragma = NULL;
    if (sqlite3_prepare_v2(db, pragma_sql, -1, &pragma, NULL) != SQLITE_OK) {
        fprintf(stderr, "sqlite_insert pragma: %s\n", sqlite3_errmsg(db));
        return SQLITE_ERROR;
    }

    char       col_names[MAX_COLS][64];
    PragmaType col_types[MAX_COLS];
    int        col_pk[MAX_COLS];
    size_t     col_count = 0;

    // PRAGMA table_info columns: cid, name, type, notnull, dflt_value, pk
    while (sqlite3_step(pragma) == SQLITE_ROW) {
        if (col_count >= MAX_COLS) break;
        const char *name = (const char *)sqlite3_column_text(pragma, 1);
        const char *type = (const char *)sqlite3_column_text(pragma, 2);
        int         pk   = sqlite3_column_int(pragma, 5);
        snprintf(col_names[col_count], sizeof(col_names[0]), "%s", name);
        col_types[col_count] = parse_pragma_type(type);
        col_pk[col_count]    = pk;
        col_count++;
    }
    sqlite3_finalize(pragma);

    if (value_count > col_count) {
        fprintf(stderr, "sqlite_insert: more values (%zu) than columns (%zu)\n",
                value_count, col_count);
        return SQLITE_ERROR;
    }

    // --- 2. Build INSERT SQL, skipping pk columns ---
    // We skip pk columns from both the column list and value binding,
    // so the caller should not include pk values in their values array.
    char sql[1024];
    size_t pos = 0;
    pos += snprintf(sql + pos, sizeof(sql) - pos, "INSERT INTO %s (", table);

    size_t bind_count = 0;
    for (size_t i = 0; i < col_count; i++) {
        if (col_pk[i]) continue;
        if (bind_count > 0)
            pos += snprintf(sql + pos, sizeof(sql) - pos, ", ");
        pos += snprintf(sql + pos, sizeof(sql) - pos, "%s", col_names[i]);
        bind_count++;
    }

    pos += snprintf(sql + pos, sizeof(sql) - pos, ") VALUES (");

    for (size_t i = 0; i < bind_count; i++) {
        if (i > 0)
            pos += snprintf(sql + pos, sizeof(sql) - pos, ", ");
        pos += snprintf(sql + pos, sizeof(sql) - pos, "?");
    }
    pos += snprintf(sql + pos, sizeof(sql) - pos, ");");

    // --- 3. Prepare and bind ---
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite_insert prepare: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    size_t val_idx = 0;
    for (size_t i = 0; i < col_count; i++) {
        if (col_pk[i]) continue;
        if (val_idx >= value_count) break;

        int bind_pos = (int)val_idx + 1; // sqlite bind is 1-indexed
        const void *v = values[val_idx];

        if (!v) {
            sqlite3_bind_null(stmt, bind_pos);
        } else {
            switch (col_types[i]) {
                case PRAGMA_INT:
                    sqlite3_bind_int64(stmt, bind_pos, *(const sqlite3_int64 *)v);
                    break;
                case PRAGMA_REAL:
                    sqlite3_bind_double(stmt, bind_pos, *(const double *)v);
                    break;
                case PRAGMA_BLOB:
                    // value should point to a sqlite_blob struct
                    // not implemented — fall through to TEXT
                case PRAGMA_TEXT:
                    sqlite3_bind_text(stmt, bind_pos, *(const char **)v,
                                      -1, SQLITE_STATIC);
                    break;
            }
        }
        val_idx++;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) rc = SQLITE_OK;
    else fprintf(stderr, "sqlite_insert step: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
    return rc;
}

// ======================== Pretty Print ========================

void sqlite_pretty_print(
    const SQLiteTable *t,
    const SQLitePrettyPrintOptions *opt,
    char *out,
    size_t out_cap
) {
    SQLitePrettyPrintOptions default_opt = {0};
    if (!opt) opt = &default_opt;

    size_t pos = 0;

    if (t->row_count == 0) {
        appendf(out, out_cap, &pos, "(no rows)\n");
        return;
    }

    size_t col_limit = (opt->max_columns && opt->max_columns < t->col_count)
        ? opt->max_columns : t->col_count;
    size_t row_limit = (opt->max_rows && opt->max_rows < t->row_count)
        ? opt->max_rows : t->row_count;

    // Header
    for (size_t c = 0; c < col_limit; c++) {
        write_cell_padded(out, out_cap, &pos, t->col_names[c], effective_width(c, t, opt));
        appendf(out, out_cap, &pos, " ");
    }
    appendf(out, out_cap, &pos, "\n");

    // Separator
    for (size_t c = 0; c < col_limit; c++) {
        size_t w = effective_width(c, t, opt);
        for (size_t i = 0; i < w; i++) appendf(out, out_cap, &pos, "-");
        appendf(out, out_cap, &pos, " ");
    }
    appendf(out, out_cap, &pos, "\n");

    // Rows
    for (size_t r = 0; r < row_limit; r++) {
        for (size_t c = 0; c < col_limit; c++) {
            write_cell_padded(out, out_cap, &pos, t->rows[r][c], effective_width(c, t, opt));
            appendf(out, out_cap, &pos, " ");
        }
        appendf(out, out_cap, &pos, "\n");
    }
}

// ======================= Get Cell's Value =====================

const char *sqlite_table_get(
    const SQLiteTable *t,
    size_t row_index,
    const char *column_name
) {
    if (!t || !t->col_names || !t->rows || row_index >= t->row_count)
        return NULL;

    // Find the column index
    size_t col = 0;
    for (; col < t->col_count; col++) {
        if (strcmp(t->col_names[col], column_name) == 0)
            break;
    }
    if (col >= t->col_count)
        return NULL; // Column not found

    return t->rows[row_index][col];
}

