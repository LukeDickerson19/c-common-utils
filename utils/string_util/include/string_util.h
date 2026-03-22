
#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <stddef.h> // for size_t
#include <stdlib.h> // for malloc, free, realloc
#include <stdbool.h> // for bool type
#include <stddef.h> // for ptrdiff_t

// for ssize_t used by utf8proc
#if defined(_WIN32) || defined(_WIN64)
    #include <basetsd.h>
    typedef SSIZE_T ssize_t;
#else
    #include <sys/types.h>
    #include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


///////////////////////////// Structs and Enums //////////////////////////

/** MemoryAllocationProcedure enum
 * 
 * The grow_capacity() and shrink_capacity() string util internal static functions implement whichever MemoryAllocationProcedure enum val a user chooses for a String struct. Default option is DEFAULT_MEM_PROC (see below)..
 * Options:
 *   - MEM_LINEAR:
 *         grow or shrink to always use the minimum amount of memory required
 *         to store the current string's text
 *   - MEM_TRAILING:
 *         grow to match text expansion, but do not shrink
 *   - MEM_DOUBLE:
 *         allocate double the initial string size, and double/halve the memory
 *         allocation each time the text outgrows the cap or shrinks to less than half the memory allocated
 *   - MEM_FIXED: fix the memory allocated to the string, if a string modifying function causes the text to outgrow the fixed memory it that function will fail and return an error code
 */
typedef enum {
    MEM_LINEAR,
    MEM_TRAILING,
    MEM_DOUBLE,
    MEM_FIXED
} MemoryAllocationProcedure;
#define DEFAULT_MEM_PROC MEM_LINEAR

/** String struct
 * 
 * UTF-8 compatible dynamic string struct
 * 
 * Struct Members:
 *   - text:                 pointer to heap-allocated memory containing the null terminated string
 *   - len:                  number of UTF-8 characters (aka "UTF-8 code points", aka "runes")
 *   - bytes:                number of bytes the current text occupies (s.bytes <= s.cap)
 *   - cap:                  total bytes of memory capacity allocated for this string to grow and shrink (s.cap >= s.bytes + 1 for null terminator, so s->cap = opts->cap + 1)
 *   - allocation_procedure: the specified procedure for how memory is allocated for when the dynamic text size grows and shrinks. See StringOptions for default value and valid options
 * 
 * Important:
 *   - Don't modify struct members directly, use the provided functions
 *   - Clean up with free_string() to avoid memory leak
 */
typedef struct String {
    char   *text;
    size_t  len;
    size_t  bytes;
    size_t  cap;
    MemoryAllocationProcedure allocation_procedure;
} String;


/** Buffer struct
 *
 * Represents a character buffer used for char array formatting functions.
 * Can wrap both stack-allocated and heap-allocated arrays. No UTF-8 normalization support
 *
 * Members:
 *   text - pointer to the underlying character array storing the string.
 *   cap  - total allocated capacity of the text array (in bytes).
 *   pos  - current write position within the buffer; new data is appended here.
 */
typedef struct Buffer {
    char *text; // text char array
    size_t cap; // memory capacity allocated for text
    size_t pos; // current position of buffer
} Buffer;


////////////////////////////// Memory Functions ///////////////////////////


/** str()
 * 
 * Creates a new String for `text`.
 *
 * Examples:
 *   String *s1 = str("Hello, world!");
 *   String *s2 = str(fmt(buf, "Name: %s, Age: %d", name, age));
 *
 * @param text   format string (must not be NULL)
 * @param opts   optional arguments
 * @return       pointer to initialized String struct (returns NULL on failure)
 * 
 * StringOptions: Optional Args for String struct
 *   - allocation_procedure:
 *         allocation_procedure specifies how to allocate memory for this String. It defaults
 *         to MemoryAllocationProcedure.MEM_LINEAR enum which grows and shrinks the string's
 *         memory allocation with the text size as the string is updated
 *   - cap:
 *         cap is the memory capacity in bytes to allocate to this string. byte_cap
 *         defaults to (size_t)-1. Only one cap, byte_cap or rune_cap, can be specified
 *         at a time
 */
typedef struct StringOptions {
    MemoryAllocationProcedure allocation_procedure;
    size_t cap;
} StringOptions;
#define DEFAULT_STRING_OPTIONS \
    .allocation_procedure = DEFAULT_MEM_PROC, \
    .cap = (size_t)-1
String *_str(const char *text, const StringOptions *opts);
#define str(text, ...) \
    _str((text), &(StringOptions){ DEFAULT_STRING_OPTIONS, ##__VA_ARGS__ })


/** str_free()
 * 
 * Frees the memory of one or multiple String structs and their struct members
 * given as a variadic list of String pointers, and resets their fields.
 *
 * Example usage:
 *     str_free(a, b, c);
 *
 * @param ...   one or more pointers to String structs to free.
 */
void _str_free(String ***list, size_t count);
#define str_free(...) \
    _str_free((String**[]){__VA_ARGS__}, \
    sizeof((String**[]){__VA_ARGS__}) / sizeof(String**))


/** str_clone()
 * 
 * Creates a deep copy of the given String.
 * Allocates a new heap buffer and copies the contents.
 *
 * @param src  source string to clone (must not be NULL)
 * @return     pointer to a new String with the same contents as src (returns NULL on failure)
 */
String *str_clone(
    const String *src
);


/** str_info()
 * 
 * Formats String struct metadata and memory footprint into a buffer.
 *
 * @param label    A descriptive name prefix for identifying the string instance.
 * @param s        Pointer to the String structure to inspect.
 * @param out      The destination Buffer struct to receive the formatted text.
 * @return         The number of characters written (standard snprintf behavior).
 */
void str_info(
    const String *s,
    Buffer *out
);

////////////////////////////// Mutation Functions /////////////////////////


/** str_append()
 * 
 * Appends the char array `suffix` to the end of `s`.
 * Automatically resizes `s` if necessary (doubles capacity repeatedly).
 *
 * @param s       destination string to append to (must be valid)
 * @param suffix  char array to append (must be null terminated)
 * @return 0 on success, -1 if reallocation failed
 */
int str_append(
    String *s,
    const char *suffix
);


/** str_prepend()
 * 
 * Prepends the char array `suffix` to the end of `s`.
 * Automatically resizes `s` if necessary (doubles capacity repeatedly).
 *
 * @param prefix  char array to prepend (must be null terminated)
 * @param s       destination string to prepend to (must be valid)
 * @return 0 on success, -1 on failure
 */
int str_prepend(
    const char *prefix,
    String *s
);


/** str_concat()
 * 
 * Concatenates multiple strings from a NULL-terminated array into one target string.
 * The result is built in the string at index `opts->output_index` (default: 0).
 * Automatically resizes the target string if necessary (doubles capacity repeatedly).
 * If `opts->sep` is provided, its contents are inserted **between** each string in the concatenation.
 *
 * @param s_list     NULL-terminated array of pointers to String structs (must contain at least one valid string).
 * @param opts       optional configuration (may be NULL → defaults: output_index = 0, free_others = false, sep = NULL).
 *                   - .output_index: index of the string that will hold the concatenated result
 *                   - .sep:          pointer to a String to insert between each string (can be NULL)
 * @return 0 on success, -1 on invalid input (NULL array/pointers, invalid output_index) or reallocation failure
 */
typedef struct {
    size_t output_index;
    String *sep;
} ConcatOptions;
#define DEFAULT_CONCAT_OPTIONS .output_index = 0, .sep = NULL
int _str_concat(String **s_list, const size_t count, const ConcatOptions *opts);
// this macro exists to simulate optional args in C
// #define str_concat(s_list, ...) _str_concat((s_list), sizeof(s_list) / sizeof((s_list)[0]), &(ConcatOptions){ DEFAULT_CONCAT_OPTIONS, ##__VA_ARGS__ })
#define str_concat(s_list, ...) \
    _str_concat( \
        (s_list), \
        sizeof(s_list) / sizeof((s_list)[0]), \
        &(ConcatOptions){ DEFAULT_CONCAT_OPTIONS, ##__VA_ARGS__ } \
    )


/** str_to_upper()
 * 
 * Converts all characters in the string to uppercase in place.
 * Non-alphabetic characters are left unchanged.
 *
 * @param s  string to convert (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_to_upper(
    String *s
);


/** str_to_lower()
 * 
 * Converts all characters in the string to lowercase in place.
 * Non-alphabetic characters are left unchanged.
 *
 * @param s  string to convert (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_to_lower(
    String *s
);


/** str_insert()
 * 
 * Inserts the contents of `sub` into `s` at the given index.
 * Automatically resizes `dst` if necessary (doubles capacity repeatedly).
 *
 * @param s           string to insert into (must be valid)
 * @param sub         string to insert (must be valid)
 * @param rune_index  position in `s` where `sub` should be inserted
 *                    (0 ≤ index ≤ s->len; inserting at s->len appends)
 * @return 0 on success, -1 on invalid input or reallocation failure
 */
int str_insert(
    String *s,
    const String *sub,
    size_t rune_index
);


/** str_replace()
 * 
 * Replaces occurrences of `old_sub` in `s` with `new_sub`.
 *
 * @param s        string to modify (must not be NULL)
 * @param old_sub  substring to replace (must not be NULL)
 * @param new_sub  substring to insert in place of old_sub (must not be NULL)
 * @param mode     replacement mode:
 *                   "first" (default) - replace first occurrence
 *                   "last"            - replace last occurrence
 *                   "all"             - replace all occurrences
 * @return 0 on success, -1 on invalid input or memory allocation failure
 */
int str_replace(
    String *s,
    const String *old_sub,
    const String *new_sub,
    const char *mode
);


/** str_repeat()
 * 
 * Repeats s->text n times, either in-place or into a caller-owned buffer.
 *
 * @param s    source string to repeat (must not be NULL)
 * @param n    number of repetitions (0 -> empty string)
 * @param ...  optional RepeatOptions:
 *               .text_buffer  - caller-owned buffer to write result into (default: NULL)
 *               .buffer_size  - size of text_buffer in bytes, required if text_buffer is set
 *
 * @return 0 on success, -1 on failure
 *
 * Behavior:
 *   - If text_buffer is NULL -> modifies s in place
 *   - If text_buffer is set  -> writes repeated string into text_buffer, s is unchanged
 *   - If text_buffer is set but buffer_size is not -> prints error and returns -1
 *   - If text_buffer is too small to hold result   -> prints error and returns -1
 *
 * Notes:
 *   - text_buffer must be at least s->bytes * n + 1 bytes
 *   - UTF-8 safe: operates on bytes, rune count is preserved
 *
 * Examples:
 *   str_repeat(s, 3);                                                  // in place
 *   str_repeat(s, 3, .text_buffer = buf, .buffer_size = sizeof(buf)); // to buffer
 */
typedef struct RepeatOptions {
    char *text_buffer;
    size_t buffer_size;
} RepeatOptions;
#define DEFAULT_REPEAT_OPTIONS \
    .text_buffer = NULL, \
    .buffer_size = (size_t)-1
int _str_repeat(String *s, size_t n, RepeatOptions *opts);
#define str_repeat(s, n, ...) \
    _str_repeat(s, n, &(RepeatOptions){ DEFAULT_REPEAT_OPTIONS, ##__VA_ARGS__ })


/** str_remove()
 * 
 * Removes a portion of the string starting at `i` and spanning `n` UTF-8 runes.
 *
 * @param s    string to modify (must not be NULL)
 * @param i    starting rune index to remove (0 ≤ i < s->len)
 * @param n    number of runes to remove
 * @return     0 on success, -1 on invalid input
 *
 * Notes:
 *   - If i >= s->len, does nothing and returns 0
 */
int str_remove(
    String *s,
    size_t start,
    size_t len
);


/** str_trim()
 * 
 * Removes whitespace from both ends of the string in place.
 *
 * @param s  string to trim (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_trim(
    String *s
);


/** str_trim_left()
 * 
 * Removes whitespace from the start (left) of the string in place.
 *
 * @param s  string to trim (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_trim_left(
    String *s
);


/** str_trim_right()
 * 
 * Removes whitespace from the end (right) of the string in place.
 *
 * @param s  string to trim (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_trim_right(
    String *s
);


/** str_clear()
 * 
 * Clears all text from the string and shrinks its memory allocation if applicable to the strings memory allocation procedure.
 *
 * @param s  string to clear (must not be NULL)
 * @return   0 on success, -1 if string is NULL
 */
int str_clear(
    String *s
);


/** str_overwrite()
 * 
 * Completely overwrites a string with new text, updating its length, byte count,
 * and memory allocation as needed.
 *
 * @param s        string to overwrite (must not be NULL)
 * @param new_text new text to assign (must not be NULL)
 * @return         0 on success, -1 on failure
 */
int str_overwrite(
    String *s,
    const char *new_text
);


////////////////////////////// Query Functions ////////////////////////////


/** str_equals()
 * 
 * Returns true if both strings contain identical characters.
 *
 * @param a  first string (must not be NULL)
 * @param b  second string (must not be NULL)
 * @return   true if equal, false otherwise
 */
bool str_equals(
    const String *a,
    const String *b
);


/** str_is_empty()
 * 
 * Returns true if the string has zero length.
 */
bool str_is_empty(
    const String *s
);


/** str_starts_with()
 * 
 * Returns true if the string starts with the given prefix.
 * @param s      the string to check
 * @param prefix the prefix to test
 */
bool str_starts_with(
    const String *s,
    const String *prefix
);


/** str_ends_with()
 * 
 * Returns true if the string ends with the given suffix.
 * @param s      the string to check
 * @param suffix the suffix to test
 */
bool str_ends_with(
    const String *s,
    const String *suffix
);


/** str_contains()
 * 
 * Returns true if `s` contains `substr`, using naive search.
 *
 * @param s       string to search in (must not be NULL)
 * @param substr  substring to search for (must not be NULL)
 * @return        true if `substr` is found within `s`, false otherwise
 *
 * Notes:
 *   - More efficient than the naive O(n*m) search for long strings or repeated patterns.
 *   - Allocates a temporary table of size substr->len (freed before returning).
 */
bool str_contains(
    const String *s,
    const String *substr
);


/** str_count()
 * 
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
size_t str_count(
    const String *s,
    const String *substr
);


/** str_index_of()
 * 
 * Returns the index of the first or last occurrence of `substr` in `s`.
 *
 * @param s       string to search in (must not be NULL)
 * @param substr  substring to search for (must not be NULL)
 * @param mode    "first" (default) -> first occurrence
 *                "last"            -> last occurrence
 * @return        index of occurrence, or -1 if not found
 */
size_t str_index_of(
    const String *s,
    const String *substr,
    const char *mode
);


/** str_indices_of()
 * 
 * Returns all indices of occurrences of `substr` in `s`.
 *
 * @param s        string to search in (must not be NULL)
 * @param substr   substring to search for (must not be NULL)
 * @param out_len  pointer to size_t that will receive number of occurrences found
 * @return         dynamically allocated array of size_t indices (must be freed by caller),
 *                 or NULL if none found or invalid input
 */
size_t* str_indices_of(
    const String *s,
    const String *substr,
    size_t *out_len
);


////////////////////////////// Extract Functions //////////////////////////


/** str_split()
 * 
 * Splits a string into substrings using a single character delimiter.
 *
 * @param s         string to split
 * @param delim     delimiter string
 * @param out_count pointer to size_t that will receive the number of substrings
 * @return          dynamically allocated array of String pointers (must be freed with a loop + str_free)
 */
String **str_split(
    const String *s,
    const String *delim,
    size_t *out_count
);


/** str_slice()
 * Copies a substring slice of String `s` at the rune index range [start, end)
 * into either the caller-owned `opts->text_buffer`, or modifying `s` in place.
 *
 * @param s      source string (must not be NULL)
 * @param start  starting UTF-8 rune index (inclusive)
 * @param end    ending UTF-8 rune index (exclusive)
 * @param ...    optional SliceOptions:
 *                 .text_buffer  - caller-owned buffer to write result into (default: NULL)
 *                 .buffer_size  - size of text_buffer in bytes, required if text_buffer is set
 * @return       0 on success, -1 on error
 *
 * Behavior:
 *   - If text_buffer is NULL -> modifies s in place
 *   - If text_buffer is set  -> writes slice into text_buffer, s is unchanged
 *   - If text_buffer is set but buffer_size is not -> prints error and returns -1
 *   - If text_buffer is too small to hold result   -> prints error and returns -1
 *   - If start > end after wrapping                -> returns -1
 *
 * Index wrapping (python-style):
 *   - Negative indices wrap from the end: -1 = last rune, -2 = second to last, etc.
 *   - Indices beyond s->len are wrapped with modulus
 *   - Multiples of s->len (e.g. 0, s->len, 2*s->len) are treated as s->len for end,
 *     and 0 for start, allowing end to reach the full length of the string
 *   - If start > end after wrapping -> returns -1
 *
 * Notes:
 *   - text_buffer must be at least byte length of slice + 1 bytes
 *   - UTF-8 safe: slices on rune boundaries, not bytes
 *   - Time complexity: O(n) where n = end - start
 *
 * Examples:
 *   str_slice(s, 0, 5);                                                       // in place
 *   str_slice(s, 0, 5, .text_buffer = buf, .buffer_size = sizeof(buf));       // to buffer
 *   str_slice(s, -3, -1, .text_buffer = buf, .buffer_size = sizeof(buf));     // negative indices
 */
typedef struct SliceOptions {
    char  *text_buffer;
    size_t buffer_size;
} SliceOptions;
#define DEFAULT_SLICE_OPTIONS \
    .text_buffer = NULL, \
    .buffer_size = (size_t)-1
int _str_slice(
    String *s,
    ssize_t start,
    ssize_t end,
    SliceOptions *opts
);
#define str_slice(s, start, end, ...) \
    _str_slice(s, start, end, &(SliceOptions){ DEFAULT_SLICE_OPTIONS, ##__VA_ARGS__ })


///////////////////////////// Char Array Formatting ///////////////////////


/** fmt()
 * 
 * fmt() is a convenience macro used to format char arrays. It requires passing a pre-created buffer managed by the caller, so use multiple buffers if nesting fmt() calls, or calling fmt() multiple times on one line so they don't interfere with each other.
 * Example usage:
 *     char *buf1[128];
 *     char *buf2[buf_size]; // Example buffer sized at runtime. C99 allows pre-defined stack buffers to have runtime determined sizes because you can use Variable Length Arrays (VLAs). function's arg
 *     printf("%s %s\n", fmt(buf1, "A"), fmt(buf2, "B"));
 * 
 * @param buf      pointer to a char array
 * @param fmt_text text to format and write into buf->text
 * @param ...      printf-style format string followed by any values to substitute
 * @return         pointer to the formatted string, buf
 */
char *fmt(
    Buffer *buf,
    const char *fmt_text,
    ...
);


/** fmt_append()
 * 
 * Appends the src char array (plus formatting) to dst buffer, updates the *pos pointer to the new end (null terminator position) of the char array, and returns the number of bytes that would have been written on success (from snprintf() function).
 * 
 * @param buf      pointer to the destination Buffer struct
 * @param pos      position in dst to append to
 * @param fmt_text text to format and write into buf->text + buf->pos
 * @param ...      printf-style string formatting values to substitute into src
 * @return         number of bytes written, or (size_t)-1 on failure
 */
size_t fmt_append(
    Buffer *buf,
    const char *fmt_text,
    ...
);


///////////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
}
#endif
#endif // STRING_UTIL_H


