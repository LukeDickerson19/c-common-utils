#include "sqlite_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h> // SIZE_MAX

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

// Returns the display width of a UTF-8 string in characters.
// Used only to decide whether truncation is needed — padding is
// handled by sqlite3_mprintf's %! flag, so we no longer need this
// for spacing math.
static size_t utf8_char_count(const char *s) {
    if (!s) return 4; // "NULL"
    size_t count = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; ) {
        if      (*p < 0x80)           { p += 1; }
        else if ((*p & 0xE0) == 0xC0) { p += 2; }
        else if ((*p & 0xF0) == 0xE0) { p += 3; }
        else if ((*p & 0xF8) == 0xF0) { p += 4; }
        else                          { p += 1; } // invalid byte, skip
        count++;
    }
    return count;
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

// Writes a single table cell into `out`, left-justified and padded to
// `width` *characters* (not bytes).  Truncates with "..." if the value
// is wider than `width`.
//
// The heavy lifting is done by sqlite3_mprintf with the non-standard
// "!" (alternate-form-2) flag, which makes width and precision operate
// in UTF-8 characters rather than bytes.  This replaces the old manual
// display_len() + space-padding loop entirely.
static void write_cell_padded(
    char *out,
    size_t cap,
    size_t *pos,
    const char *s,
    size_t width
) {
    if (!s) s = "NULL";

    size_t char_count = utf8_char_count(s);
    char *cell;

    if (char_count > width) {
        // Truncate: show as many characters as fit, then "..."
        if (width <= 3) {
            // Not enough room for any content — just fill with dots
            cell = sqlite3_mprintf("%-!*.*s", (int)width, (int)width, "...");
        } else {
            // "%!.*s" — precision in characters (via !) reads char count
            // from the argument, truncating the string cleanly on a
            // character boundary rather than a byte boundary.
            cell = sqlite3_mprintf("%!.*s...", (int)(width - 3), s);
        }
    } else {
        // "%-!*s" — left-justify (the '-' flag), pad to `width`
        // characters (not bytes) using the '!' flag.
        // Without '!', a 3-byte UTF-8 character would consume 3 units
        // of the width field, producing misaligned columns.
        cell = sqlite3_mprintf("%-!*s", (int)width, s);
    }

    assert(cell && "sqlite3_mprintf OOM");
    appendf(out, cap, pos, "%s", cell);
    sqlite3_free(cell);
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
        // Track width in characters, not bytes, so column sizing is
        // correct for headers and data containing multi-byte UTF-8.
        t->col_widths[i] = utf8_char_count(name);
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        char **row = arena_alloc(&a, col_count * sizeof(char *));

        for (int c = 0; c < col_count; c++) {
            const unsigned char *txt = sqlite3_column_text(stmt, c);
            const char *val = txt ? (const char *)txt : "NULL";
            row[c] = arena_strdup(&a, val);
            // Use character count (not byte length) so that a column
            // containing multi-byte values gets a width wide enough to
            // display them, not artificially inflated by byte overhead.
            size_t char_w = utf8_char_count(val);
            if (char_w > t->col_widths[c]) t->col_widths[c] = char_w;
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

    // Whether we're eliding columns / rows at all
    int elide_cols = opt->max_columns && opt->max_columns < t->col_count;
    int elide_rows = opt->max_rows    && opt->max_rows    < t->row_count;

    // How many real columns/rows to show on each side of the "..."
    // e.g. max_columns=4 → 2 left, 2 right; max_columns=3 → 2 left, 1 right
    size_t col_limit  = elide_cols ? opt->max_columns : t->col_count;
    size_t col_left   = elide_cols ? (col_limit + 1) / 2 : t->col_count;
    size_t col_right  = elide_cols ? col_limit / 2        : 0;

    size_t row_limit  = elide_rows ? opt->max_rows : t->row_count;
    size_t row_top    = elide_rows ? (row_limit + 1) / 2 : t->row_count;
    size_t row_bottom = elide_rows ? row_limit / 2        : 0;

    // The "..." elision column is always 3 chars wide
    #define ELISION_COL_WIDTH 3

    // Iterates over visible column indices, calling `body` with each.
    // Inserts the elision column in the middle when eliding.
    // We use a macro here to avoid duplicating the col-iteration logic
    // across header, separator, and every data row.
    #define FOR_EACH_COL(body)                                              \
        do {                                                                \
            for (size_t c = 0; c < col_left; c++)  { body; }              \
            if (elide_cols) {                                               \
                size_t c = SIZE_MAX; (void)c; /* sentinel — not a real col */ \
                body;                                                       \
            }                                                               \
            for (size_t c = t->col_count - col_right;                      \
                 c < t->col_count; c++) { body; }                          \
        } while (0)

    // Helper: write one cell in the elision column
    #define ELISION_CELL() \
        write_cell_padded(out, out_cap, &pos, "...", ELISION_COL_WIDTH)

    // --- Header ---
    FOR_EACH_COL({
        if (c == SIZE_MAX) {
            ELISION_CELL();
        } else {
            write_cell_padded(out, out_cap, &pos,
                t->col_names[c], effective_width(c, t, opt));
        }
        appendf(out, out_cap, &pos, " ");
    });
    appendf(out, out_cap, &pos, "\n");

    // --- Separator ---
    FOR_EACH_COL({
        size_t w = (c == SIZE_MAX) ? ELISION_COL_WIDTH : effective_width(c, t, opt);
        for (size_t i = 0; i < w; i++) appendf(out, out_cap, &pos, "-");
        appendf(out, out_cap, &pos, " ");
    });
    appendf(out, out_cap, &pos, "\n");

    // --- Rows (top half, elision row if needed, bottom half) ---
    #define PRINT_ROW(r)                                                    \
        do {                                                                \
            FOR_EACH_COL({                                                  \
                if (c == SIZE_MAX) {                                        \
                    ELISION_CELL();                                         \
                } else {                                                    \
                    write_cell_padded(out, out_cap, &pos,                   \
                        t->rows[r][c], effective_width(c, t, opt));        \
                }                                                           \
                appendf(out, out_cap, &pos, " ");                          \
            });                                                             \
            appendf(out, out_cap, &pos, "\n");                             \
        } while (0)

    for (size_t r = 0; r < row_top; r++)
        PRINT_ROW(r);

    if (elide_rows) {
        // Elision row: "..." in the first cell, blank in the rest
        FOR_EACH_COL({
            size_t w = (c == SIZE_MAX) ? ELISION_COL_WIDTH : effective_width(c, t, opt);
            write_cell_padded(out, out_cap, &pos,
                (c == col_left - 1 || c == 0) ? "..." : "", w);
            // ^^^ put "..." only in the first real column so it reads
            // like a pandas elision row, not a wall of dots
            appendf(out, out_cap, &pos, " ");
        });
        appendf(out, out_cap, &pos, "\n");
    }

    for (size_t r = t->row_count - row_bottom; r < t->row_count; r++)
        PRINT_ROW(r);

    #undef ELISION_COL_WIDTH
    #undef FOR_EACH_COL
    #undef ELISION_CELL
    #undef PRINT_ROW
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

