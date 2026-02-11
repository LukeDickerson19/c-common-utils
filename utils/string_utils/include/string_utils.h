
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

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
    char   *text;    // The actual string data (heap allocated, null-terminated)
    size_t  len;     // Number of characters (not counting the '\0')
    size_t  cap;     // Allocated buffer size in bytes
} String;

/**
 * Creates a new String by copying the given null-terminated C-string.
 * Allocates memory with an initial capacity of roughly 2×length + 1.
 *
 * @param text  null-terminated source string (must not be NULL)
 * @return      initialized String struct (exits program on allocation failure)
 */
String str(const char *text);


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
int _append(String *dst, String *suffix, const AppendOptions *opts);
// this macro exists to simulate optional args in C
#define append(dst, suffix, ...) _append((dst), (suffix), &(AppendOptions){ DEFAULT_APPEND_OPTIONS, ##__VA_ARGS__})
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
int _prepend(String *prefix, String *dst, const PrependOptions *opts);
// this macro exists to simulate optional args in C
#define prepend(prefix, dst, ...) _prepend((prefix), (dst), &(PrependOptions){ DEFAULT_PREPEND_OPTIONS, ##__VA_ARGS__})


/**
 * Concatenates multiple strings from a NULL-terminated array into one target string.
 * The result is built in the string at index `opts->output_index` (default: 0).
 * Automatically resizes the target string if necessary (doubles capacity repeatedly).
 * If `opts->free_others` is true, all strings except the one at `output_index` are freed
 * after their contents have been copied.
 *
 * @param str_lst    NULL-terminated array of pointers to String structs (must contain at least one valid string).
 * @param opts       optional configuration (may be NULL → defaults: output_index = 0, free_others = false).
 *                   - .output_index: index of the string that will hold the concatenated result
 *                   - .free_others:  if true, free all other strings after copying their contents
 * @return 0 on success, -1 on invalid input (NULL array/pointers, invalid output_index) or reallocation failure
 */
typedef struct {
    int output_index; // index in str_lst of string to concatinate output to, defaults to 0 (the first string)
    bool free_others;  // boolean flag to free all other strings in str_lst except the one at output_index, defaults to false
} ConcatOptions;
#define DEFAULT_CONCAT_OPTIONS .output_index = 0, .free_others = false
int _concat(String *str_lst[], const ConcatOptions *opts);
// this macro exists to simulate optional args in C
#define concat(str_lst, ...) _concat((str_lst), &(ConcatOptions){ DEFAULT_CONCAT_OPTIONS, ##__VA_ARGS__})


/**
 * Frees the dynamically allocated text buffer of a single String
 * and resets the struct fields to a safe empty state.
 *
 * @param string  pointer to the String struct to free
 * @return 0 on success, -1 if string was NULL
 */
int free_string(String *string);


/**
 * Frees the dynamically allocated text buffers of multiple String structs
 * given in a NULL-terminated array of pointers, and resets their fields.
 *
 * Typical usage:
 *     String a = str("hello"), b = str("world");
 *     String* list[] = {&a, &b, NULL};
 *     free_strings(list);
 *
 * @param string_list  NULL-terminated array of String pointers
 * @return 0 on success, -1 if string_list is NULL
 */
int free_strings(String *string_list[]);


#endif


