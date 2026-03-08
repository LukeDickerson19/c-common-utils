#include "string_util.h"
#include <string.h>     // strlen, strcat, memcpy, memmove
#include <stdbool.h> // for bool
#include <ctype.h>  // for toupper, tolower
#include <stdio.h> // for va_list
#include <stdarg.h> // for [tbd]
#include <stdint.h> // for SIZE_MAX

///////////////////////// String Struct Constructor ///////////////////////

String *str(const char *fmt, ...) {
    if (!fmt) fmt = "";

    va_list args;
    va_start(args, fmt);

    // First, determine the required length
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        va_end(args);
        return NULL; // formatting error
    }

    size_t len = (size_t)needed;
    if (len > (SIZE_MAX - 1) / 2)
        return NULL;
    size_t cap = 2 * len + 1; // double for growth + null terminator
    char *content = malloc(cap);
    if (!content) {
        va_end(args);
        return NULL; // allocation failure
    }

    vsnprintf(content, cap, fmt, args);
    va_end(args);

    // Allocate string struct
    String *s = malloc(sizeof(String));
    if (!s) {
        free(content);
        return NULL;
    }
    *s = (String){
        .text = content,
        .len  = len,
        .cap  = cap
    };
    return s;
}

////////////////////////////// Memory Functions ///////////////////////////

static int grow_capacity(String *s, size_t min_len) {
    if (!s) return -1;

    size_t required = min_len + 1;
    if (required <= s->cap)
        return 0;

    size_t new_cap = s->cap;

    // Safety: if cap somehow became 0
    if (new_cap == 0)
        new_cap = 1;

    while (new_cap < required)
        new_cap *= 2;

    char *new_text = realloc(s->text, new_cap);
    if (!new_text)
        return -1;

    s->text = new_text;
    s->cap  = new_cap;
    return 0;
}

static int shrink_capacity(String *s) {
    if (!s) return -1;

    size_t min_cap = s->len + 1;

    // Only shrink if we're using less than 25%
    if (s->len >= s->cap / 4)
        return 0;

    size_t new_cap = s->cap;

    while (new_cap / 2 >= min_cap && s->len < new_cap / 4)
        new_cap /= 2;

    if (new_cap == s->cap)
        return 0;

    char *new_text = realloc(s->text, new_cap);
    if (!new_text)
        return -1;

    s->text = new_text;
    s->cap  = new_cap;
    return 0;
}

void _str_free(String ***list, size_t count) {

    // do nothing for empty list of 0 count
    if (!list || count == 0) return;

    // Iterate over each String pointer
    for (size_t i = 0; i < count; i++) {

        // Get the pointer to the pointer of the String
        String **p = list[i];

        // Skip if the pointer is NULL or the caller's pointer is already NULL
        if (!p || !*p) continue;

        // Dereference once to get the actual String struct
        String *s = *p;

        // Free the dynamically allocated text inside the struct
        if (s->text) {
            free(s->text);
            s->text = NULL;  // defensive reset (not strictly necessary)
        }

        // Free the String struct itself
        free(s);

        // Set the caller's pointer to NULL to prevent a dangling pointer
        *p = NULL;
    }
}

String *str_clone(const String *src) {

    // Return NULL if source is invalid
    if (src == NULL || src->text == NULL) return NULL;

    // Use str() init function to allocate a new String
    String *copy = str("%.*s", (int)src->len, src->text);
    if (!copy) return NULL; // allocation failed
    return copy;
}

////////////////////////////// Mutation Functions /////////////////////////

int str_append(String *s, const char *suffix) {
    if (s == NULL || s->text == NULL || suffix == NULL)
        return -1;

    size_t suffix_len = strlen(suffix);

    // Resize dynamic array if needed
    size_t needed = s->len + suffix_len + 1;
    if (needed > s->cap) {
        int rc = grow_capacity(s, needed);
        if (rc != 0)
            return -1;
    }

    // append suffix characters to s->text
    memcpy(s->text + s->len, suffix, suffix_len);
    s->text[s->len + suffix_len] = '\0';
    s->len += suffix_len;
    return 0;
}

int str_prepend(const char *prefix, String *s) {
    if (s == NULL || s->text == NULL || prefix == NULL)
        return -1;

    size_t prefix_len = strlen(prefix);

    // Resize if needed
    // Resize dynamic array if needed
    size_t needed = s->len + prefix_len + 1;
    if (needed > s->cap) {
        int rc = grow_capacity(s, needed);
        if (rc != 0)
            return -1;
    }

    // Shift existing s->text to the right (including null terminator)
    memmove(s->text + prefix_len, s->text, s->len + 1);

    // Copy prefix to the beginning
    memcpy(s->text, prefix, prefix_len);
    s->len += prefix_len;

    return 0;
}

int _str_concat(String **s_list, const size_t count, const ConcatOptions *opts) {
    if (count == 0 || s_list == NULL || s_list[0] == NULL)
        return -1;

    // Default values if opts is NULL (though the macro always provides one)
    int output_index = (opts != NULL) ? opts->output_index : 0;
    String *sep      = (opts != NULL) ? opts->sep : NULL;
    size_t sep_len   = (sep && sep->text) ? sep->len : 0;

    if (output_index < 0 || output_index >= (int)count)
        return -1; // invalid output index

    // Build the result in s_list[output_index]
    String *result = s_list[output_index];

    // Compute total length including separators
    size_t num_strings = 0;
    for (size_t i = 0; i < count; i++)
        if (s_list[i]) num_strings++;

    // Resize result string if necessary
    size_t total_len = 0;
    for (size_t i = 0; i < count; i++)
        total_len += s_list[i]->len;
    total_len += sep_len * (num_strings - 1);
    if (total_len + 1 > result->cap) {
        if (grow_capacity(result, total_len + 1) != 0)
            return -1;
    }

    // If output_index != 0, copyt result's text to a tmp char*
    char *tmp = NULL;
    size_t tmp_len = 0;
    if (output_index != 0 && result->len > 0) {
        tmp_len = result->len;
        tmp = malloc(tmp_len + 1);
        if (!tmp) return -1;
        memcpy(tmp, result->text, tmp_len + 1);
    }

    // Append all other strings
    // and free them immediately if requested
    char *dst = result->text;
    size_t offset = 0;
    bool first_appended = false;
    for (size_t i = 0; i < count; i++) {
        if (!s_list[i]) continue;

        if (i == (size_t)output_index) {
            if (tmp) {
                memcpy(dst + offset, tmp, tmp_len);
                offset += tmp_len;
                free(tmp);
                tmp = NULL;
            } else {
                offset += result->len; // already valid
            }
            first_appended = true;
            continue;
        }

        // Only insert separator if something has been appended
        if (first_appended && sep_len > 0) {
            memcpy(dst + offset, sep->text, sep_len);
            offset += sep_len;
        }

        memcpy(dst + offset, s_list[i]->text, s_list[i]->len);
        offset += s_list[i]->len;

        first_appended = true;
    }

    dst[offset] = '\0';
    result->len = offset;

    return 0;
}

int str_to_upper(String *s) {
    if (s == NULL || s->text == NULL) return -1;

    for (size_t i = 0; i < s->len; i++) {
        s->text[i] = (char)toupper((unsigned char)s->text[i]);
    }
    return 0;
}

int str_to_lower(String *s) {
    if (s == NULL || s->text == NULL) return -1;

    for (size_t i = 0; i < s->len; i++) {
        s->text[i] = (char)tolower((unsigned char)s->text[i]);
    }
    return 0;
}

int str_insert(String *dst, const String *substr, size_t index) {
    if (!dst || !dst->text || !substr || !substr->text)
        return -1;
    if (index > dst->len)  // allow insert at end
        return -1;

    // Calculate new length
    size_t new_len = dst->len + substr->len;

    // Resize if needed
    if (new_len + 1 > dst->cap) {
        int rc = grow_capacity(dst, new_len + 1);
        if (rc != 0) return -1;
    }

    // Shift existing content right
    memmove(dst->text + index + substr->len, dst->text + index, dst->len - index + 1); // +1 to move null terminator

    // Copy substring into place
    memcpy(dst->text + index, substr->text, substr->len);

    dst->len = new_len;
    return 0;
}

int str_replace(String *dst, const String *old_sub, const String *new_sub, const char *mode) {
    if (!dst || !dst->text || !old_sub || !old_sub->text || !new_sub || !new_sub->text)
        return -1;

    if (old_sub->len == 0 || dst->len < old_sub->len) return -1;  // invalid

    if (!mode) mode = "first"; // default mode

    size_t pos = 0;  // position of match
    size_t i;

    // Helper: find first or last occurrence
    if (strcmp(mode, "first") == 0) {
        for (i = 0; i <= dst->len - old_sub->len; i++) {
            if (strncmp(dst->text + i, old_sub->text, old_sub->len) == 0) {
                pos = i;
                break;
            }
        }
        if (i > dst->len - old_sub->len) return 0; // nothing to replace
    } else if (strcmp(mode, "last") == 0) {
        for (i = dst->len - old_sub->len + 1; i-- > 0;) {
            if (strncmp(dst->text + i, old_sub->text, old_sub->len) == 0) {
                pos = i;
                break;
            }
        }
        if (i == (size_t)-1) return 0; // nothing to replace
    } else if (strcmp(mode, "all") == 0) {
        // We'll handle "all" after computing new total length
        // Count occurrences first
        size_t count = 0;
        for (i = 0; i <= dst->len - old_sub->len; i++) {
            if (strncmp(dst->text + i, old_sub->text, old_sub->len) == 0) {
                count++;
                i += old_sub->len - 1; // move past this occurrence
            }
        }
        if (count == 0) return 0; // nothing to replace

        size_t new_len = dst->len + count * (new_sub->len - old_sub->len);
        if (new_len + 1 > dst->cap) {
            if (grow_capacity(dst, new_len + 1) != 0) return -1;
        }

        // Rebuild string with replacements
        char *buf = malloc(new_len + 1);
        if (!buf) return -1;

        size_t src_idx = 0, dst_idx = 0;
        while (src_idx < dst->len) {
            if (src_idx <= dst->len - old_sub->len &&
                strncmp(dst->text + src_idx, old_sub->text, old_sub->len) == 0) {
                memcpy(buf + dst_idx, new_sub->text, new_sub->len);
                dst_idx += new_sub->len;
                src_idx += old_sub->len;
            } else {
                buf[dst_idx++] = dst->text[src_idx++];
            }
        }
        buf[dst_idx] = '\0';

        free(dst->text);
        dst->text = buf;
        dst->len = new_len;
        dst->cap = new_len + 1; // optional: could keep old cap if larger

        shrink_capacity(dst);
        return 0;
    } else {
        return -1; // invalid mode
    }

    // Single replacement ("first" or "last")
    size_t new_len = dst->len + (new_sub->len - old_sub->len);
    if (grow_capacity(dst, new_len) != 0)
        return -1;

    // Shift remainder to make space / remove old_sub
    if (new_sub->len != old_sub->len) {
        memmove(dst->text + pos + new_sub->len,
                dst->text + pos + old_sub->len,
                dst->len - pos - old_sub->len + 1); // include null terminator
    }

    memcpy(dst->text + pos, new_sub->text, new_sub->len);
    dst->len = new_len;

    shrink_capacity(dst);
    return 0;
}

int str_repeat(String *s, const size_t n) {
    if (!s || !s->text) return -1; // invalid input

    // Return empty string if n == 0
    if (n == 0) {
        s->text[0] = '\0';
        s->len = 0;
        s->cap = 1;
        char *tmp = realloc(s->text, s->cap);
        if (!tmp) return -1;
        s->text = tmp;
        return 0;
    }

    if (s->len != 0 && n > SIZE_MAX / s->len) return -1;
        
    // Resize dynamic array if needed
    size_t old_len = s->len;
    size_t new_len = old_len * n;
    size_t needed = new_len + 1;
    if (needed > s->cap) {
        int rc = grow_capacity(s, needed);
        if (rc != 0)
            return -1;
    }

    // repeat text n times
    for (size_t i = 1; i < n; i++)
        memcpy(s->text + i * old_len, s->text, old_len);
    s->text[new_len] = '\0';
    s->len = new_len;
    return 0;
}

int str_remove(String *s, size_t start, size_t len) {
    if (!s || !s->text) return -1;
    if (start >= s->len) return 0; // nothing to remove

    if (start + len > s->len) {
        len = s->len - start;
    }

    memmove(s->text + start, s->text + start + len, s->len - start - len + 1); // include null terminator
    s->len -= len;

    // Shrink capacity if string is now much smaller
    if (s->cap > 2 * s->len + 1)
        shrink_capacity(s); // use your shrink function

    return 0;
}

static inline bool is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

int str_trim(String *s) {
    if (!s || !s->text) return -1;
    str_trim_left(s);
    str_trim_right(s);
    return 0;
}

int str_trim_left(String *s) {
    if (!s || !s->text) return -1;

    size_t start = 0;
    while (start < s->len && is_space(s->text[start])) {
        start++;
    }

    if (start > 0) {
        memmove(s->text, s->text + start, s->len - start + 1); // include null terminator
        s->len -= start;
    }

    return 0;
}

int str_trim_right(String *s) {
    if (!s || !s->text) return -1;

    if (s->len == 0) return 0;

    size_t end = s->len;
    while (end > 0 && is_space(s->text[end - 1])) {
        end--;
    }

    s->len = end;
    s->text[s->len] = '\0';

    return 0;
}

////////////////////////////// Query Functions ////////////////////////////

bool str_equals(const String *a, const String *b) {
    if (!a || !b) return false;

    // Quick length check
    if (a->len != b->len) return false;

    // If both empty, they are equal
    if (a->len == 0) return true;

    // Compare raw bytes (faster than strcmp because we know length)
    return memcmp(a->text, b->text, a->len) == 0;
}

bool str_is_empty(const String *s) {
    if (s == NULL) return true; // treat NULL as empty
    return s->len == 0;
}

bool str_starts_with(const String *s, const String *prefix) {
    if (s == NULL || prefix == NULL) return false;
    if (prefix->len > s->len) return false;
    return memcmp(s->text, prefix->text, prefix->len) == 0;
}

bool str_ends_with(const String *s, const String *suffix) {
    if (s == NULL || suffix == NULL) return false;
    if (suffix->len > s->len) return false;
    return memcmp(s->text + s->len - suffix->len, suffix->text, suffix->len) == 0;
}

bool str_contains(const String *s, const String *substr) {
    return str_index_of(s, substr, "first") != -1;
}

size_t str_count(const String *s, const String *substr) {
    if (!s || !substr || !s->text || !substr->text) return 0;
    if (substr->len == 0 || s->len < substr->len) return 0;

    size_t count = 0;
    for (size_t i = 0; i <= s->len - substr->len; ) {
        if (strncmp(s->text + i, substr->text, substr->len) == 0) {
            count++;
            i += substr->len; // move past this occurrence
        } else {
            i++;
        }
    }
    return count;
}

int str_index_of(const String *s, const String *substr, const char *mode) {
    if (!s || !s->text || !substr || !substr->text) return -1;
    if (substr->len == 0 || s->len < substr->len) return -1;

    if (!mode) mode = "first";

    if (strcmp(mode, "first") == 0) {
        const char *text = s->text;
        const char *pat  = substr->text;
        int n = s->len;
        int m = substr->len;
        char first = pat[0];
        for (int i = 0; i <= n - m; i++) {
            if (text[i] == first && memcmp(text + i, pat, m) == 0)
                return i;
        }
    } else if (strcmp(mode, "last") == 0) {
        const char *text = s->text;
        const char *pat  = substr->text;
        int n = s->len;
        int m = substr->len;
        char first = pat[0];
        for (int i = n - m; i >= 0; i--) {
            if (text[i] == first && memcmp(text + i, pat, m) == 0)
                return i;
        }
    } else {
        return -1; // invalid mode
    }

    return -1; // not found
}

int* str_indices_of(const String *s, const String *substr, size_t *count) {

    // return NULL for invalid arg inputs
    if (!s || !s->text || !substr || !substr->text || !count)
        return NULL;

    // set count to 0 and return NULL to represent empty list if s is empty
    *count = 0;
    if (substr->len == 0 || s->len < substr->len)
        return NULL;

    // Allocate worst-case array
    int *indices = malloc(s->len * sizeof(int));
    if (!indices) return NULL;

    // fill worst-case sized array with each substr match in s
    for (int i = 0; i <= s->len - substr->len; i++) {
        if (strncmp(s->text + i, substr->text, substr->len) == 0) {
            indices[(*count)++] = i;
            i += substr->len - 1; // skip past this match
        }
    }

    // Trim array to exact size (O(1) for shrinking)
    // NOTE: if substr was never found, realloc(..., 0) returns NULL
    int *result = realloc(indices, *count * sizeof(int));
    if (!result && *count > 0)
        return indices; // if realloc fails, just return the untrimmed array
    return result;
}

////////////////////////////// Extract Functions //////////////////////////

String **str_split(const String *s, char delim, size_t *out_count) {
    if (!s || !s->text || !out_count) return NULL;

    *out_count = 0;
    size_t n = s->len;
    if (n == 0) return NULL;

    // Count pieces
    size_t pieces = 1;
    for (size_t i = 0; i < n; i++) {
        if (s->text[i] == delim) pieces++;
    }

    // Allocate array of String pointers
    String **result = malloc(pieces * sizeof(String *));
    if (!result) return NULL;

    size_t start = 0;
    size_t idx = 0;
    for (size_t i = 0; i <= n; i++) {
        if (i == n || s->text[i] == delim) {
            size_t len = i - start;

            // Allocate String struct
            String *part = malloc(sizeof(String));
            if (!part) {
                for (size_t j = 0; j < idx; j++)
                    str_free(&result[j]);
                free(result);
                return NULL;
            }

            // Allocate text buffer
            part->text = malloc(len + 1);
            if (!part->text) {
                free(part);
                for (size_t j = 0; j < idx; j++)
                    str_free(&result[j]);
                free(result);
                return NULL;
            }

            memcpy(part->text, s->text + start, len);
            part->text[len] = '\0';

            part->len = len;
            part->cap = len + 1;

            result[idx++] = part;
            start = i + 1;
        }
    }

    *out_count = pieces;
    return result;
}

String *str_slice(const String *s, size_t start, size_t end) {
    if (!s || !s->text) return NULL;

    if (start >= s->len) start = s->len; // ensure start <= s->len
    if (end > s->len) end = s->len; // ensure end <= s->len
    if (end < start) end = start; // ensures new_len >= 0
    size_t new_len = end - start;

    // Use the str() constructor to allocate a new String with the substring
    String *result = str("%.*s", (int)new_len, s->text + start);
    // str() already returns NULL if allocation fails
    return result;
}

///////////////////////////////////////////////////////////////////////////



