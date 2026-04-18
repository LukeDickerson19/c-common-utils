#include "sqlite_util.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // Open database
    sqlite3 *db = NULL;
    const char *filename = "sqlite_example.db";
    if (sqlite3_open(filename, &db) != SQLITE_OK) {
        fprintf(stderr, "open failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    printf("\nUsing sqlite database:\n    %s\n", filename);

    char query[256];
    const char *table = "users";

    // Drop and create table
    snprintf(query, sizeof(query), "DROP TABLE IF EXISTS %s;", table);
    printf("\nExecuting SQL:\n    %s\n\n", query);
    sqlite_execute(db, query, NULL);

    snprintf(query, sizeof(query),
        "CREATE TABLE %s ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " name TEXT,"
        " age INTEGER,"
        " city TEXT);",
        table);
    printf("Executing SQL:\n    %s\n\n", query);
    sqlite_execute(db, query, NULL);

    // Insert rows — mix of ASCII and multi-byte UTF-8 names/cities.
    //
    // UTF-8 byte counts vs character counts for the non-ASCII values:
    //   "Ãlice"   — 1 precomposed 2-byte char + 4 ASCII = 6 bytes, 5 chars
    //   "Björn"   — ö is 2 bytes; total 6 bytes, 5 chars
    //   "Søren"   — ø is 2 bytes; total 6 bytes, 5 chars
    //   "日本語"   — 3 CJK chars × 3 bytes = 9 bytes, 3 chars (each renders ~2 wide)
    //   "Ünité"   — Ü + é are 2 bytes each; total 7 bytes, 5 chars
    //   "Mos̈cow"  — s̈ is a combining sequence; 2 bytes extra; 6 bytes, 6 chars
    //   "São Paulo"— ã is 2 bytes; total 10 bytes, 9 chars
    //   "Zürich"  — Ü is 2 bytes; total 7 bytes, 6 chars
    //   "Tōkyō"   — two macron o's × 2 bytes; 7 bytes, 5 chars
    //   "Réykjavík"— é + í are 2 bytes each; 11 bytes, 9 chars
    sqlite3_int64 ages[] = { 30,       24,       42,       24,       35,       28     };
    const char *names[]  = { "Ãlice",  "Björn",  "Søren",  "日本語", "Ünité",  "Dave" };
    const char *cities[] = { "São Paulo", "Zürich", "Tōkyō", "Réykjavík", "London", "Mos̈cow" };

    for (size_t i = 0; i < 6; i++) {
        const void *vals[] = { &names[i], &ages[i], &cities[i] };
        sqlite_insert(db, table, vals, 3);
    }

    // First SELECT — all rows, including all UTF-8 content
    SQLiteTable t = {0};
    snprintf(query, sizeof(query), "SELECT id, name, age, city FROM %s;", table);
    printf("Executing SQL:\n    %s\n\n", query);
    sqlite_execute(db, query, &t);

    char buf[16 * 1024];
    printf("Full table (UTF-8 names and cities):\n");
    sqlite_pretty_print(&t, NULL, buf, sizeof(buf));
    printf("%s\n", buf);
    sqlite_table_free(&t);

    // Test max_column_width truncation with multi-byte content.
    // If truncation cuts on bytes instead of characters, a 2- or 3-byte
    // sequence can be split, producing garbage or a corrupt character.
    SQLiteTable t_trunc = {0};
    snprintf(query, sizeof(query), "SELECT id, name, age, city FROM %s;", table);
    sqlite_execute(db, query, &t_trunc);

    SQLitePrettyPrintOptions trunc_opt = {
        .max_column_width = 5   // narrow enough to force truncation on multi-byte values
        // .max_rows = 4
        // .max_columns = 2
    };
    printf("With max_column_width = %d (truncation must not split UTF-8 sequences):\n",
           (int)trunc_opt.max_column_width);
    sqlite_pretty_print(&t_trunc, &trunc_opt, buf, sizeof(buf));
    printf("%s\n", buf);
    sqlite_table_free(&t_trunc);

    // Delete rows by age
    const int age_to_delete = 24;
    snprintf(query, sizeof(query), "DELETE FROM %s WHERE age = %d;", table, age_to_delete);
    printf("Executing SQL:\n    %s\n\n", query);
    sqlite_execute(db, query, NULL);

    // Second SELECT — remaining rows after delete
    SQLiteTable t2 = {0};
    snprintf(query, sizeof(query), "SELECT id, name, age, city FROM %s;", table);
    printf("Executing SQL:\n    %s\n\n", query);
    sqlite_execute(db, query, &t2);

    printf("After DELETE WHERE age = %d:\n", age_to_delete);
    sqlite_pretty_print(&t2, NULL, buf, sizeof(buf));
    printf("%s\n", buf);

    // sqlite_table_get with a UTF-8 value
    const char *col_name = "name";
    size_t row_index = 0;
    const char *name = sqlite_table_get(&t2, row_index, col_name);
    if (name)
        printf("Value at row %zu, column '%s': %s\n", row_index, col_name, name);
    else
        printf("Value not found at row %zu, column '%s'\n", row_index, col_name);

    col_name = "city";
    row_index = 1;
    const char *city = sqlite_table_get(&t2, row_index, col_name);
    if (city)
        printf("Value at row %zu, column '%s': %s\n", row_index, col_name, city);
    else
        printf("Value not found at row %zu, column '%s'\n", row_index, col_name);

    col_name = "age";
    row_index = 1;
    const char *age_str = sqlite_table_get(&t2, row_index, col_name);
    if (age_str) {
        int age = atoi(age_str);
        printf("Value at row %zu, column '%s': %d\n", row_index, col_name, age);
    } else {
        printf("Value not found at row %zu, column '%s'\n", row_index, col_name);
    }

    sqlite_table_free(&t2);
    sqlite3_close(db);
    return 0;
}
