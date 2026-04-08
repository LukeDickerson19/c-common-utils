#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uthash.h"

/* Hash Map:

    Key/value pairs can be dynamically added/removed from a uthash map.
    
    The data type of the keys and values must be hardcoded in the MapEntry struct,
    and the return type of the map_get() function.

    If you change the key type, you must also update:
        - how keys are allocated
        - how they are freed
        - which HASH_ADD / HASH_FIND macro is used

    depending on the situation, it might be better to:
        - give the map ownership of the data (aka let the map handle the allocating and freeing of the map values' memory) as is done here. btw this code could be optimized further to pass an int and a string to map_put() instead of a MapValue so the called doesn't need to create/free MapValues themselves. Right now its duplicating the MapValues which is probably a bad idea.
        - give the caller of the map functions ownership/responsibility of the map values' data by just giving the map entry a pointer

    many C programmers seem to prefer to build hashmaps themselves:
    https://www.reddit.com/r/C_Programming/comments/1mztfc0/what_hashmap_library_do_you_use_for_your_projects/

    Hashtable lib Performance Comparison:
    https://jacksonallan.github.io/c_cpp_hash_tables_benchmark/

*/

/* value stored in the hash map */
typedef struct {
    int i; // example integer data
    char *s; // string of any length cause its dynamically allocated
} MapValue;

/* Define the hash table entry */
typedef struct {
    char *key;           // string key (string of any length cause its dynamically allocated)
    MapValue value;      // struct as value
    UT_hash_handle hh;   // makes this struct hashable
} MapEntry;

/* Hash map container */
typedef struct {
    MapEntry *entries;      // hash table head
    size_t size;            // number of key/value pairs
} HashMap;

/* map_init: initialize an empty map */
void map_init(HashMap *map) {
    map->entries = NULL;
    map->size = 0;
}

/* map_put: add/update a key/value pair

    Time Complexity:
        Average: O(1) - Hash lookup + optional insert into a bucket.
        Worst case: O(n) - All keys collide into the same bucket.

    Memory Complexity:
        O(1) per new key - Allocates exactly one MapEntry struct when inserting.

*/
void map_put(HashMap *map, const char *key, MapValue value) {
    MapEntry *e;
    HASH_FIND_STR(map->entries, key, e);
    if (!e) {

        // allocate entry
        e = malloc(sizeof(*e));
        if (!e) exit(1);
        e->key = malloc(strlen(key) + 1); // allocate exact-length key string, ISO C replacement for strdup()
        if (!e->key) exit(1);
        strcpy(e->key, key);

        // add key pointing to map entry to uthash map
        HASH_ADD_KEYPTR(hh, map->entries, e->key, strlen(e->key), e);

        // increment map's count
        map->size++;
    }

    // Update existing value
    e->value.i = value.i;
    if (e->value.s) free(e->value.s);
    if (value.s) {
        e->value.s = malloc(strlen(value.s) + 1);
        if (!e->value.s) exit(1);
        strcpy(e->value.s, value.s);
    } else {
        e->value.s = NULL;
    }
}

/* map_get: lookup the value associated with a key, return NULL if key doesnt exist in map

    Time Complexity:
        Average: O(1)
        Worst case: O(n)

    Memory Complexity:
        O(1) - no allocation performed

*/
MapValue *map_get(HashMap *map, const char *key) {
    MapEntry *e;
    HASH_FIND_STR(map->entries, key, e);
    return e ? &e->value : NULL;
}

/* map_remove: remove a single key/value pair from the map

    Time Complexity:
        Average: O(1)
        Worst case: O(n)

    Memory Complexity:
        O(1) - Frees exactly one MapEntry struct if the key exists.

*/
void map_remove(HashMap *map, const char *key) {
    MapEntry *e;
    HASH_FIND_STR(map->entries, key, e);
    if (e) {
        HASH_DEL(map->entries, e); // remove the entry from the uthash internal data structures
        if (e->value.s) free(e->value.s); // free the struct value's dynamic length string
        free(e->key); // free the key's dynamic length string
        free(e); // free the map entry
        map->size--; // decrement map's count
        /* NOTE:
            HASH_DEL must be called before free() because HASH_DEL removes the entry from the
            internal hash table data structures that uthash maintains. If you called free(e)
            first, the memory for e would be deallocated, but HASH_DEL would then try to access
            e->hh and other fields to unlink it from the hash table.
        */
    }
}

/* map_clear: Remove and free all key/value pairs in the map.

    Time Complexity:
        O(n) - Every MapEntry must be visited and freed.

    Memory Complexity:
        O(1) auxiliary memory - Frees O(n) total memory, but uses only constant extra space.

*/
void map_clear(HashMap *map) {
    MapEntry *curr, *tmp;
    HASH_ITER(hh, map->entries, curr, tmp) {
        HASH_DEL(map->entries, curr); // remove the entry from the uthash internal data structures
        if (curr->value.s) free(curr->value.s); // free the struct value's dynamic length string
        free(curr->key); // free the key's dynamic length string
        free(curr);
    }
    map->entries = NULL;
    map->size = 0;
}


int main(void) {

    // Initialize map
    HashMap map;
    map_init(&map);

    // Add key/value pairs
    MapValue v1 = {42, strdup("hello")};
    MapValue v2 = {99, strdup("world")};
    MapValue v3 = {7, strdup("foo")};
    map_put(&map, "apple",  v1);
    map_put(&map, "banana", v2);
    map_put(&map, "orange", v3);

    // Get individual key/value pairs
    printf("banana = %d\n", map_get(&map, "banana"));
    printf("apple = %d\n", map_get(&map, "apple"));
    
    // Update key/value pair
    MapValue v4 = {8, "foobar"};
    map_put(&map, "orange", v4);
    
    // Iterate over all key/value pairs
    printf("Iterating over map:\n");
    MapEntry *curr, *tmp;
    HASH_ITER(hh, map.entries, curr, tmp) {
        printf("    key = %s, i = %d, s = %s\n", curr->key, curr->value.i, curr->value.s);
    }
    /* NOTE:
        The first argument (hh) tells the HASH_ITER macro which field of the struct is the hash handle.
        Since MapEntry has UT_hash_handle hh;, the macro can access it internally.
        If hh weren’t in the struct, uthash macros would fail with an “unknown identifier” error.
    */

    printf("map size = %zu\n", map.size);

    // Remove individual key/value pair
    map_remove(&map, "apple");
    printf("map size after removing \"apple\" = %zu\n", map.size);

    // Clear all key/value pairs
    free(v1.s);
    free(v2.s);
    free(v3.s);
    map_clear(&map);
    printf("map size after clear = %zu\n", map.size);

    return 0;
}
