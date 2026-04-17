#include "sqlite_util.h"
#include <stdio.h>
#include <stdlib.h>  // For atoi, atoll, strtol, etc.

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
        " age INTEGER);",
        table);
    printf("Executing SQL:\n    %s\n\n", query);
    sqlite_execute(db, query, NULL);

    // Insert rows
    sqlite3_int64 ages[] = {  30,      24,    42,      24    };
    const char *names[]  = { "Alice", "Bob", "Carol", "Dave" };
    for (size_t i = 0; i < 4; i++) {
        const void *vals[] = { &names[i], &ages[i] };
        sqlite_insert(db, table, vals, 2);
    }

    // First SELECT
    SQLiteTable t = {0};
    snprintf(query, sizeof(query), "SELECT id, name, age FROM %s;", table);
    printf("Executing SQL:\n    %s\n\n", query);
    sqlite_execute(db, query, &t);

    char buf[16 * 1024];
    sqlite_pretty_print(&t, NULL, buf, sizeof(buf));
    printf("%s\n", buf);
    sqlite_table_free(&t);

    // Delete rows
    const int age_to_delete = 24;
    snprintf(query, sizeof(query), "DELETE FROM %s WHERE age = %d;", table, age_to_delete);
    printf("Executing SQL:\n    %s\n\n", query);
    sqlite_execute(db, query, NULL);

    // Second SELECT
    SQLiteTable t2 = {0};
    snprintf(query, sizeof(query), "SELECT id, name, age FROM %s;", table);
    printf("Executing SQL:\n    %s\n\n", query);
    sqlite_execute(db, query, &t2);

    // Uncomment to test print options:
    // SQLitePrettyPrintOptions opt = {
    //     .max_rows = 1,
    //     .max_columns = 2,
    //     .max_column_width = 4
    // };
    // sqlite_pretty_print(&t2, &opt, buf, sizeof(buf));
    sqlite_pretty_print(&t2, NULL, buf, sizeof(buf));
    printf("%s\n", buf);
    
    // Print specific values
    char * col_name = "name";
    size_t row_index = 1;
    const char *name = sqlite_table_get(&t2, row_index, col_name);
    if (name) {
        printf("Value at row %d, column '%s': %s\n", row_index, col_name, name);
    } else {
        printf("Value not found at row %d, column '%s'\n", row_index, col_name);
    }
    col_name = "age";
    row_index = 1;
    const char *age_str = sqlite_table_get(&t2, row_index, col_name);
    if (age_str) {
        // Cast to int and print
        int age = atoi(age_str);
        printf("Value at row %d, column '%s': %d\n", row_index, col_name, age);
    } else {
        printf("Value not found at row %d, column '%s'\n", row_index, col_name);
    }
    
    sqlite_table_free(&t2);
    sqlite3_close(db);
    return 0;
}

