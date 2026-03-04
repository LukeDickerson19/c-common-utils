#include "string_util.h"
#include <stdlib.h>     // malloc, realloc, free
#include <string.h>     // strlen, strcat, memcpy, memmove
#include <stdbool.h> // for bool
#include <ctype.h>  // for toupper, tolower
#include <stdio.h> // for va_list
#include <stdarg.h> // for [tbd]

String str(const char *fmt, ...) {
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
        exit(1); // formatting error
    }

    size_t len = (size_t)needed;
    size_t cap = 2 * len + 1; // double for growth + null terminator
    char *content = malloc(cap);
    if (!content) {
        va_end(args);
        exit(1); // allocation failure
    }

    vsnprintf(content, cap, fmt, args);
    va_end(args);

    return (String){
        .text = content,
        .len  = len,
        .cap  = cap
    };
}

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

int _str_free(String **s_list, size_t count) {
    if (s_list == NULL || count == 0)
        return -1;
    int status = 0; // 0 = success, -1 = at least one "problem"
    for (size_t i = 0; i < count; i++) {
        String *s = s_list[i];
        if (!s) continue; // nothing to free
        if (s->text) {
            free(s->text);
            s->text = NULL;
        }
        s->len = 0;
        s->cap = 0;
        s_list[i] = NULL;
    }
    return status; // 0 if all good, -1 if any issues flagged
}

String str_clone(const String *src) {
    if (src == NULL || src->text == NULL) {
        // Return empty string if source is invalid
        return str("");
    }

    // Allocate a new String with the same capacity
    char *new_text = malloc(src->cap);
    if (!new_text) {
        exit(1); // handle allocation failure
    }

    // Copy contents including null terminator
    memcpy(new_text, src->text, src->len + 1);

    return (String){
        .text = new_text,
        .len  = src->len,
        .cap  = src->cap
    };
}

int _str_append(String *dst, String *suffix, const AppendOptions *opts) {
    if (dst == NULL || dst->text == NULL || suffix == NULL || suffix->text == NULL)
        return -1;

    // Default if opts is NULL (though macro always supplies one)
    bool free_suffix = (opts != NULL) ? opts->free_suffix : false;

    // Resize dynamic array if needed
    size_t needed = dst->len + suffix->len + 1;
    if (needed > dst->cap) {
        int rc = grow_capacity(dst, needed);
        if (rc != 0)
            return -1;
    }

    // Perform append
    // Using memcpy is usually faster/safer than strcat here
    memcpy(dst->text + dst->len, suffix->text, suffix->len);
    dst->text[dst->len + suffix->len] = '\0';
    dst->len += suffix->len;

    // Clean up suffix if requested
    if (free_suffix)
        str_free(suffix);

    return 0;
}

int _str_prepend(String *prefix, String *dst, const PrependOptions *opts) {
    if (dst == NULL || dst->text == NULL || prefix == NULL || prefix->text == NULL)
        return -1;

    // Default if opts is NULL
    bool free_prefix = (opts != NULL) ? opts->free_prefix : false;

    // Resize if needed
    // Resize dynamic array if needed
    size_t needed = dst->len + prefix->len + 1;
    if (needed > dst->cap) {
        int rc = grow_capacity(dst, needed);
        if (rc != 0)
            return -1;
    }

    // Shift existing content to the right (including null terminator)
    memmove(dst->text + prefix->len, dst->text, dst->len + 1);

    // Copy prefix to the beginning
    memcpy(dst->text, prefix->text, prefix->len);
    dst->len += prefix->len;

    // Clean up prefix if requested
    if (free_prefix)
        str_free(prefix);

    return 0;
}

int _str_concat(String **s_list, const size_t count, const ConcatOptions *opts) {
    if (count == 0 || s_list == NULL || s_list[0] == NULL)
        return -1;

    // Default values if opts is NULL (though the macro always provides one)
    int output_index = (opts != NULL) ? opts->output_index : 0;
    bool free_others = (opts != NULL) ? opts->free_others : false;
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

        // Free right after copying if requested
        if (free_others) {
            str_free(s_list[i]);
            // Note: we don't need to NULL the pointer here if you don't plan to reuse
            // the array afterward — but it's harmless and safer to do so
            s_list[i] = NULL;  // optional but recommended to prevent misuse
        }


    }

    dst[offset] = '\0';
    result->len = offset;

    return 0;
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

bool str_contains(const String *s, const String *substr) {
    // uses the Knuth–Morris–Pratt (KMP) string search algorithm for O(n + m) time complexity
    // https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm

    if (s == NULL || substr == NULL) return false;
    if (substr->len == 0) return true;
    if (substr->len > s->len) return false;

    size_t n = s->len;
    size_t m = substr->len;

    // Build longest prefix-suffix (LPS) table
    size_t *lps = malloc(m * sizeof(size_t));
    if (!lps) return false; // allocation failure
    lps[0] = 0;
    size_t len = 0;
    for (size_t i = 1; i < m; i++) {
        while (len > 0 && substr->text[i] != substr->text[len]) {
            len = lps[len - 1];
        }
        if (substr->text[i] == substr->text[len]) {
            len++;
        }
        lps[i] = len;
    }

    size_t i = 0; // index for s
    size_t j = 0; // index for substr
    while (i < n) {
        if (s->text[i] == substr->text[j]) {
            i++;
            j++;
            if (j == m) {
                free(lps);
                return true; // match found
            }
        } else {
            if (j != 0) {
                j = lps[j - 1]; // fallback in pattern
            } else {
                i++;
            }
        }
    }

    free(lps);
    return false; // no match
}

int str_index_of(const String *s, const String *substr, const char *mode) {
    if (!s || !s->text || !substr || !substr->text) return -1;
    if (substr->len == 0 || s->len < substr->len) return -1;

    if (!mode) mode = "first";

    if (strcmp(mode, "first") == 0) {
        for (int i = 0; i <= (int)(s->len - substr->len); i++) {
            if (strncmp(s->text + i, substr->text, substr->len) == 0)
                return i;
        }
    } else if (strcmp(mode, "last") == 0) {
        for (int i = (int)(s->len - substr->len); i >= 0; i--) {
            if (strncmp(s->text + i, substr->text, substr->len) == 0)
                return i;
        }
    } else {
        return -1; // invalid mode
    }

    return -1; // not found
}

int* str_indices_of(const String *s, const String *substr, int *out_len) {
    if (!s || !s->text || !substr || !substr->text || !out_len) return NULL;
    *out_len = 0;
    if (substr->len == 0 || s->len < substr->len) return NULL;

    // Count occurrences
    int count = 0;
    for (int i = 0; i <= (int)(s->len - substr->len); i++) {
        if (strncmp(s->text + i, substr->text, substr->len) == 0) {
            count++;
            i += (int)substr->len - 1; // skip past current occurrence
        }
    }

    if (count == 0) return NULL;

    int *indices = malloc(count * sizeof(int));
    if (!indices) return NULL;

    int idx = 0;
    for (int i = 0; i <= (int)(s->len - substr->len); i++) {
        if (strncmp(s->text + i, substr->text, substr->len) == 0) {
            indices[idx++] = i;
            i += (int)substr->len - 1;
        }
    }

    *out_len = count;
    return indices;
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

    if (old_sub->len == 0) return -1;  // invalid

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

String *str_split(const String *s, char delim, size_t *out_count) {
    if (!s || !s->text || !out_count) return NULL;

    *out_count = 0;
    size_t n = s->len;
    if (n == 0) return NULL;

    // First, count how many pieces
    size_t pieces = 1;
    for (size_t i = 0; i < n; i++) {
        if (s->text[i] == delim) pieces++;
    }

    String *result = malloc(pieces * sizeof(String));
    if (!result) return NULL; // allocation failure

    size_t start = 0;
    size_t idx = 0;
    for (size_t i = 0; i <= n; i++) {
        if (i == n || s->text[i] == delim) {
            size_t len = i - start;
            char *buf = malloc(len + 1);
            if (!buf) {
                // cleanup already allocated strings
                for (size_t j = 0; j < idx; j++) str_free(&result[j]);
                free(result);
                return NULL;
            }
            memcpy(buf, s->text + start, len);
            buf[len] = '\0';
            result[idx].text = buf;
            result[idx].len  = len;
            result[idx].cap  = len + 1;
            idx++;
            start = i + 1;
        }
    }

    *out_count = pieces;
    return result;
}

bool str_equals(const String *a, const String *b) {
    if (!a || !b) return false;

    // Quick length check
    if (a->len != b->len) return false;

    // If both empty, they are equal
    if (a->len == 0) return true;

    // Compare raw bytes (faster than strcmp because we know length)
    return memcmp(a->text, b->text, a->len) == 0;
}

String str_slice(const String *s, size_t start, size_t end) {
    if (!s || !s->text || start >= end || start >= s->len)
        return str("");

    if (end > s->len)
        end = s->len;

    size_t new_len = end - start;

    String result;
    result.len = new_len;
    result.cap = new_len + 1;
    result.text = malloc(result.cap);

    if (!result.text) {
        fprintf(stderr, "str_slice: allocation failed\n");
        exit(EXIT_FAILURE);
    }

    memcpy(result.text, s->text + start, new_len);
    result.text[new_len] = '\0';

    return result;
}

String str_repeat(const String *s, size_t count) {
    if (!s || !s->text || count == 0) return str("");

    size_t new_len = s->len * count;
    String result;
    result.len = new_len;
    result.cap = new_len + 1;
    result.text = malloc(result.cap);

    if (!result.text) {
        fprintf(stderr, "str_repeat: allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < count; i++) {
        memcpy(result.text + i * s->len, s->text, s->len);
    }

    result.text[new_len] = '\0';
    return result;
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

int str_remove(String *s, size_t start, size_t len) {
    if (!s || !s->text) return -1;
    if (start >= s->len) return 0; // nothing to remove

    if (start + len > s->len) {
        len = s->len - start;
    }

    memmove(s->text + start, s->text + start + len, s->len - start - len + 1); // include null terminator
    s->len -= len;

    // Optionally shrink capacity if string is now much smaller
    if (s->cap > 2 * s->len + 1) {
        shrink_capacity(s); // use your shrink function
    }

    return 0;
}

static inline bool is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
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

int str_trim(String *s) {
    if (!s || !s->text) return -1;
    str_trim_left(s);
    str_trim_right(s);
    return 0;
}



