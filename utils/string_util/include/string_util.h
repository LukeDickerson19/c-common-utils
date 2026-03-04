
#ifndef string_util_H
#define string_util_H

#include <stddef.h> // for size_t
#include <stdlib.h> // for malloc, free, realloc, exit
#include <stdbool.h> // for bool type

/**
 * A simple dynamic string implementation.
 * 
 * Fields:
 *   - text: pointer to heap-allocated memory containing the string + '\0'
 *   - len:  number of actual characters (strlen(s.text) == s.len)
 *   - cap:  total bytes allocated (s.cap >= s.len + 1 always)
 * 
 * Important:
 *   - Never modify .text or .len directly — use the provided functions
 *   - Always initialize with str() or zero-init + manual allocation
 *   - Clean up with free_string() to avoid memory leaks
 */
typedef struct {
    char   *text;
    size_t  len;
    size_t  cap;
} String;

/**
 * Creates a new String by copying a formatted string.
 * Works like printf: you can provide a format string and optional arguments.
 * Allocates memory with an initial capacity of roughly 2×length + 1.
 *
 * Examples:
 *   String s1 = str("Hello, world!");
 *   String s2 = str("Name: %s, Age: %d", name, age);
 *
 * @param fmt   format string (must not be NULL)
 * @param ...   optional arguments for formatting
 * @return      initialized String struct (exits program on allocation failure)
 */
String str(const char *fmt, ...);

/**
 * Frees the memory of one or multiple String structs and their struct members
 * given as a variadic list of String pointers, and resets their fields.
 *
 * Example usage:
 *     str_free(&a, &b, &c);
 *
 * @param ...      One or more pointers to String structs to free.
 * @return 0 on success, -1 if no valid pointers are provided.
 */
int _str_free(String **s_list, size_t count);
#define str_free(...) \
    _str_free((String*[]){__VA_ARGS__}, \
    sizeof((String*[]){__VA_ARGS__})/sizeof(String*))

/**
 * Creates a deep copy of the given String.
 * Allocates a new heap buffer and copies the contents.
 *
 * @param src  source string to clone (must not be NULL)
 * @return     a new String with the same contents as src (exits on allocation failure)
 */
String str_clone(const String *src);


/**
 * Appends the contents of `suffix` to the end of `dst`.
 * Automatically resizes `dst` if necessary (doubles capacity repeatedly).
 * If opts->free_suffix is true, the suffix string is freed after successful append.
 *
 * @param dst     destination string to append to (must be valid)
 * @param suffix  string to append (must be valid)
 * @param opts    optional configuration (may be NULL → default: do not free suffix)
 * @return 0 on success, -1 if reallocation failed
 */
typedef struct {
    bool free_suffix;  // boolean flag to free suffix string after appending, defaults to false
} AppendOptions;
#define DEFAULT_APPEND_OPTIONS .free_suffix = false
int _str_append(String *dst, String *suffix, const AppendOptions *opts);
// this macro exists to simulate optional args in C
#define str_append(dst, suffix, ...) _str_append((dst), (suffix), &(AppendOptions){ DEFAULT_APPEND_OPTIONS, ##__VA_ARGS__})
// NOTE: __VA_ARGS__ override default print options because when they're later in the struct initialization
// The prepended "##" characters is a GNU extension that removes the comma if __VA_ARGS__ is empty. This is widely supported but not part of the C standard.


/**
 * Prepends the contents of `prefix` to the beginning of `dst`.
 * Automatically resizes `dst` if necessary and shifts existing content right.
 * If opts->free_prefix is true, the prefix string is freed after successful prepend.
 *
 * @param prefix  string to prepend (must be valid)
 * @param dst     destination string to prepend to (must be valid)
 * @param opts    optional configuration (may be NULL → default: do not free prefix)
 * @return 0 on success, -1 if reallocation failed
 */
typedef struct {
    bool free_prefix;  // boolean flag to free prefix string after prepending, defaults to false
} PrependOptions;
#define DEFAULT_PREPEND_OPTIONS .free_prefix = false
int _str_prepend(String *prefix, String *dst, const PrependOptions *opts);
// this macro exists to simulate optional args in C
#define str_prepend(prefix, dst, ...) _str_prepend((prefix), (dst), &(PrependOptions){ DEFAULT_PREPEND_OPTIONS, ##__VA_ARGS__})


/**
 * Concatenates multiple strings from a NULL-terminated array into one target string.
 * The result is built in the string at index `opts->output_index` (default: 0).
 * Automatically resizes the target string if necessary (doubles capacity repeatedly).
 * If `opts->free_others` is true, all strings except the one at `output_index` are freed
 * after their contents have been copied.
 * If `opts->sep` is provided, its contents are inserted **between** each string in the concatenation.
 *
 * @param s_list     NULL-terminated array of pointers to String structs (must contain at least one valid string).
 * @param opts       optional configuration (may be NULL → defaults: output_index = 0, free_others = false, sep = NULL).
 *                   - .output_index: index of the string that will hold the concatenated result
 *                   - .free_others:  if true, free all other strings after copying their contents
 *                   - .sep:          pointer to a String to insert between each string (can be NULL)
 * @return 0 on success, -1 on invalid input (NULL array/pointers, invalid output_index) or reallocation failure
 */
typedef struct {
    int output_index;
    bool free_others;
    String *sep;
} ConcatOptions;
#define DEFAULT_CONCAT_OPTIONS .output_index = 0, .free_others = false, .sep = NULL
int _str_concat(String **s_list, const size_t count, const ConcatOptions *opts);
// this macro exists to simulate optional args in C
// #define str_concat(s_list, ...) _str_concat((s_list), sizeof(s_list) / sizeof((s_list)[0]), &(ConcatOptions){ DEFAULT_CONCAT_OPTIONS, ##__VA_ARGS__ })
#define str_concat(s_list, ...) \
    _str_concat( \
        (s_list), \
        sizeof(s_list) / sizeof((s_list)[0]), \
        &(ConcatOptions){ DEFAULT_CONCAT_OPTIONS, ##__VA_ARGS__ } \
    )


/**
 * Returns true if the string has zero length.
 */
bool str_is_empty(const String *s);

/**
 * Returns true if the string starts with the given prefix.
 * @param s      the string to check
 * @param prefix the prefix to test
 */
bool str_starts_with(const String *s, const String *prefix);

/**
 * Returns true if the string ends with the given suffix.
 * @param s      the string to check
 * @param suffix the suffix to test
 */
bool str_ends_with(const String *s, const String *suffix);


/**
 * Converts all characters in the string to uppercase in place.
 * Non-alphabetic characters are left unchanged.
 *
 * @param s  string to convert (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_to_upper(String *s);

/**
 * Converts all characters in the string to lowercase in place.
 * Non-alphabetic characters are left unchanged.
 *
 * @param s  string to convert (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_to_lower(String *s);


/**
 * Returns true if `s` contains `substr`, using the Knuth–Morris–Pratt (KMP) algorithm
 * for efficient searching in O(n + m) time (where n = s->len, m = substr->len).
 * source: https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm
 *
 * @param s       string to search in (must not be NULL)
 * @param substr  substring to search for (must not be NULL)
 * @return        true if `substr` is found within `s`, false otherwise
 *
 * Notes:
 *   - More efficient than the naive O(n*m) search for long strings or repeated patterns.
 *   - Allocates a temporary table of size substr->len (freed before returning).
 */
bool str_contains(const String *s, const String *substr);


/**
 * Returns the index of the first or last occurrence of `substr` in `s`.
 *
 * @param s       string to search in (must not be NULL)
 * @param substr  substring to search for (must not be NULL)
 * @param mode    "first" (default) → first occurrence
 *                "last"            → last occurrence
 * @return        index of occurrence, or -1 if not found
 */
int str_index_of(const String *s, const String *substr, const char *mode);

/**
 * Returns all indices of occurrences of `substr` in `s`.
 *
 * @param s        string to search in (must not be NULL)
 * @param substr   substring to search for (must not be NULL)
 * @param out_len  pointer to int that will receive number of occurrences found
 * @return         dynamically allocated array of int indices (must be freed by caller),
 *                 or NULL if none found or invalid input
 */
int* str_indices_of(const String *s, const String *substr, int *out_len);


/**
 * Inserts the contents of `substr` into `dst` at the given index.
 * Automatically resizes `dst` if necessary (doubles capacity repeatedly).
 *
 * @param dst     destination string to insert into (must be valid)
 * @param substr  string to insert (must be valid)
 * @param index   position in `dst` where `substr` should be inserted
 *                (0 ≤ index ≤ dst->len; inserting at dst->len appends)
 * @return 0 on success, -1 on invalid input or reallocation failure
 */
int str_insert(String *dst, const String *substr, size_t index);


/**
 * Replaces occurrences of `old_sub` in `dst` with `new_sub`.
 *
 * @param dst      string to modify (must not be NULL)
 * @param old_sub  substring to replace (must not be NULL)
 * @param new_sub  substring to insert in place of old_sub (must not be NULL)
 * @param mode     replacement mode:
 *                   "first" (default) → replace first occurrence
 *                   "last"            → replace last occurrence
 *                   "all"             → replace all occurrences
 * @return 0 on success, -1 on invalid input or memory allocation failure
 *
 * Notes:
 *   - Automatically resizes `dst` if needed.
 *   - If `old_sub` is empty, function returns -1 (invalid).
 */
int str_replace(String *dst, const String *old_sub, const String *new_sub, const char *mode);


/**
 * Splits a string into substrings using a single character delimiter.
 *
 * @param s         string to split (must not be NULL)
 * @param delim     delimiter character
 * @param out_count pointer to size_t that will receive the number of substrings
 * @return          dynamically allocated array of Strings (must be freed with str_free in a loop + free())
 */
String *str_split(const String *s, char delim, size_t *out_count);


/**
 * Returns true if both strings contain identical characters.
 *
 * @param a  first string (must not be NULL)
 * @param b  second string (must not be NULL)
 * @return   true if equal, false otherwise
 */
bool str_equals(const String *a, const String *b);


/**
 * Returns a substring of `s` in the half-open range [start, end).
 *
 * @param s      source string (must not be NULL)
 * @param start  starting index (inclusive)
 * @param end    ending index (exclusive)
 * @return       new String containing the requested slice
 *
 * Behavior:
 *   - If start >= end → returns empty string
 *   - If start >= s->len → returns empty string
 *   - If end > s->len → end is clamped to s->len
 *
 * Notes:
 *   - Returned String is heap-allocated and must be freed with str_free().
 *   - Does not modify the original string.
 *   - Time complexity: O(n) where n = end - start.
 */
String str_slice(const String *s, size_t start, size_t end);


/**
 * Returns a new String consisting of `s` repeated `count` times.
 *
 * @param s      string to repeat (must not be NULL)
 * @param count  number of repetitions (0 → returns empty string)
 * @return       newly allocated String, must be freed by caller
 */
String str_repeat(const String *s, size_t count);


/**
 * Counts the number of non-overlapping occurrences of `substr` in `s`.
 *
 * @param s       string to search in (must not be NULL)
 * @param substr  substring to count (must not be NULL)
 * @return        number of occurrences (0 if none found)
 *
 * Notes:
 *   - If `substr` is empty, returns 0
 *   - Non-overlapping: "aaa" with "aa" counts as 1
 */
size_t str_count(const String *s, const String *substr);


/**
 * Removes a portion of the string starting at `start` and spanning `len` characters.
 *
 * @param s      string to modify (must not be NULL)
 * @param start  starting index to remove (0 ≤ start < s->len)
 * @param len    number of characters to remove
 * @return       0 on success, -1 on invalid input
 *
 * Notes:
 *   - If start >= s->len, does nothing and returns 0
 *   - Automatically shifts the remainder of the string left
 *   - Can shrink the allocated capacity if needed
 */
int str_remove(String *s, size_t start, size_t len);


/**
 * Removes whitespace from both ends of the string in place.
 *
 * @param s  string to trim (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_trim(String *s);

/**
 * Removes whitespace from the start (left) of the string in place.
 *
 * @param s  string to trim (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_trim_left(String *s);

/**
 * Removes whitespace from the end (right) of the string in place.
 *
 * @param s  string to trim (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_trim_right(String *s);


#endif


