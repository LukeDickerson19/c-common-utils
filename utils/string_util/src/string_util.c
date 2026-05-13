#include "string_util.h"
#include <utf8proc.h>
#include <string.h>     // strlen, strcat, memcpy, memmove
#include <stdbool.h>    // for bool
#include <ctype.h>      // for toupper, tolower
#include <stdio.h>      // for va_list
#include <stdarg.h>     // for [tbd]
#include <stdint.h>     // for SIZE_MAX


////////////////////////////// UTF-8 Functions ////////////////////////////


static char *normalize_utf8_str(
    const char *input
) {
    if (!input) return NULL;

    utf8proc_uint8_t *nfc = utf8proc_NFC((const utf8proc_uint8_t *)input);
    if (!nfc) return NULL;
    // NOTE: utf8proc_NFC returns a null-terminated string

    // Allocate memory for the normalized string
    size_t nfc_len = strlen((char *)nfc);
    char *result = malloc(nfc_len + 1);
    if (!result) {
        free(nfc);
        return NULL;
    }

    // Copy the normalized string
    memcpy(result, nfc, nfc_len + 1);
    free(nfc);
    return result;
}


static size_t count_utf8_runes(
    const char *s
) {
    /* UTF-8 Rune Byte Structure:

        In order to represent characters in languages other than english (and also emojis and other symbols)
        UTF-8 encodes each symbol as a "rune" instead of a character. Runes are stored in 1–4 bytes.

        | Rune length | First byte pattern | Continuation bytes           | Example                    |
        | ----------- | ------------------ | ---------------------------- | -------------------------- |
        | 1 byte      | `0xxxxxxx`         | none                         | ASCII `A` = 0x41           |
        | 2 bytes     | `110xxxxx`         | `10xxxxxx`                   | `é` = 0xC3 0xA9            |
        | 3 bytes     | `1110xxxx`         | `10xxxxxx 10xxxxxx`          | `漢` = 0xE6 0xBC 0xA2      |
        | 4 bytes     | `11110xxx`         | `10xxxxxx 10xxxxxx 10xxxxxx` | `😀` = 0xF0 0x9F 0x98 0x80 |

        Continuation bytes always have the form `10xxxxxx` (0x80–0xBF).
        The first byte tells how many bytes the rune takes.

        Example:
            Let’s take `😀` (U+1F600) = 0xF0 0x9F 0x98 0x80:
                Byte 1: 11110000  → indicates 4-byte rune
                Byte 2: 10011111  → continuation
                Byte 3: 10011000  → continuation
                Byte 4: 10000000  → continuation

            If you truncate the string in the middle:
            * Only `0xF0 0x9F` -> invalid
            * Need to drop it or replace with `?`
        */
    if (!s) return 0;

    size_t runes = 0;
    utf8proc_int32_t codepoint;
    const utf8proc_uint8_t *p = (const utf8proc_uint8_t *)s;
    ssize_t remaining = (ssize_t)strlen(s);

    while (remaining > 0) {
        ssize_t n = utf8proc_iterate(p, remaining, &codepoint);
        if (n <= 0) break;
        runes++;
        p += n;
        remaining -= n;
    }

    return runes;
}


static size_t rune_to_byte_index(
    const String *s,
    size_t rune_index
) {
    size_t byte_index = 0;
    utf8proc_int32_t codepoint;
    const utf8proc_uint8_t *p = (const utf8proc_uint8_t *)s->text;
    ssize_t remaining = (ssize_t)s->bytes;
    size_t current_rune = 0;

    while (remaining > 0 && current_rune < rune_index) {
        ssize_t n = utf8proc_iterate(p, remaining, &codepoint);
        if (n <= 0) break;
        byte_index += n;
        p += n;
        remaining -= n;
        current_rune++;
    }
    return byte_index;
}


static size_t byte_to_rune_index(
    const String *s,
    size_t byte_offset
) {
    // Converts a byte offset into a rune (codepoint) index.
    // e.g. in "漢字a", byte offset 6 -> rune index 2
    size_t rune_index = 0;
    const utf8proc_uint8_t *cur = (const utf8proc_uint8_t *)s->text;
    utf8proc_ssize_t remaining = (utf8proc_ssize_t)byte_offset;
    while (remaining > 0) {
        utf8proc_int32_t codepoint;
        utf8proc_ssize_t n = utf8proc_iterate(cur, remaining, &codepoint);
        if (n <= 0) break;
        rune_index++;
        cur += n;
        remaining -= n;
    }
    return rune_index;
}


static void rune_range_to_byte_range(
    const char *str,
    size_t rune_start,
    size_t rune_len,
    size_t *byte_start,
    size_t *byte_len
) {
    /* Helper function to convert rune index + length to byte offset + length */
    *byte_start = 0;
    *byte_len = 0;

    utf8proc_int32_t codepoint;
    const utf8proc_uint8_t *p = (const utf8proc_uint8_t *)str;
    ssize_t remaining = strlen(str);
    size_t current_rune = 0;
    size_t current_byte = 0;

    while (remaining > 0 && current_rune < rune_start + rune_len) {
        ssize_t n = utf8proc_iterate(p, remaining, &codepoint);
        if (n <= 0) break;
        if (current_rune == rune_start)
            *byte_start = current_byte;
        if (current_rune >= rune_start)
            *byte_len += n;
        p += n;
        current_byte += n;
        remaining -= n;
        current_rune++;
    }
}


static inline bool is_whitespace_rune(
    utf8proc_int32_t codepoint
) {
    const utf8proc_property_t *prop = utf8proc_get_property(codepoint);
    return prop->category == UTF8PROC_CATEGORY_ZS ||
           prop->category == UTF8PROC_CATEGORY_ZL ||
           prop->category == UTF8PROC_CATEGORY_ZP ||
           codepoint == ' ' || codepoint == '\t' || codepoint == '\n' ||
           codepoint == '\r' || codepoint == '\v' || codepoint == '\f';
}


////////////////////////////// Memory Functions ///////////////////////////


String *_str(
    const char *text,
    const StringOptions *opts
) {

    if (!text) text = ""; // default NULL text to empty text
    if (!opts) opts = &(StringOptions){ DEFAULT_STRING_OPTIONS }; // in case user calls _str() without str() macro

    // Normalize the string to NFC form (validating UTF-8 characters in the process):
    // In some languages (ex: French) there are multiple ways to combine UTF-8 characters to get the same resulting text. UTF-8 Normalization standardizes all such examples to the same combination so text search and comparison works properly.
    // NOTE: the text normalize_utf8_str() returns is null terminated
    char *normalized = normalize_utf8_str(text);
    if (!normalized) return NULL;

    // Get the byte and rune lengths of the normalized string
    size_t bytes = strlen(normalized);
    size_t len = count_utf8_runes(normalized);

    // Initialize the memory capacity
    size_t cap;
    if (opts->cap != (size_t)-1) {
        if (opts->cap < bytes) {
            fprintf(stderr, "STRING ERROR: requested cap of %zu is less than the bytes the text uses: %zu\n", opts->cap, bytes);
            goto fail;
        } else {
            cap = opts->cap + 1; // +1 for null terminator
        }
    } else {
        // else no cap was specified,
        // so just set the memory cap based on the bytes of the text
        if (opts->allocation_procedure == MEM_DOUBLE) {
            cap = 2 * bytes + 1; // double for MEM_DOUBLE
        } else {
            cap = bytes + 1; // minimum required for all other allocation procedures
        }
    }

    // Update text size to memory cap
    char *tmp = realloc(normalized, cap);
    if (!tmp) {
        fprintf(stderr, "STRING ERROR: Failed to realloc(normalized, cap)\n");
        goto fail;
    }
    normalized = tmp;

    // Allocate string struct
    String *s = malloc(sizeof(String));
    if (!s) {
        fprintf(stderr, "STRING ERROR: Failed to malloc(sizeof(String))\n");
        goto fail;
    }

    // Assign the normalized string and count the utf-8 runes
    s->text = normalized;
    s->bytes = bytes;
    s->cap = cap;
    s->len = len;
    s->allocation_procedure = opts->allocation_procedure;

    return s;
    fail:
        free(normalized);
        return NULL;

}


static int resize_capacity(
    String *s,
    size_t new_cap
) {
    // Resize the text memory allocation to the new_cap
    char *new_text = realloc(s->text, new_cap);
    if (!new_text) return -1;
    s->text = new_text;
    s->cap  = new_cap;
    return 0;
}


static int grow_capacity(
    String *s,
    size_t new_bytes
) {
    switch (s->allocation_procedure) {
        case MEM_LINEAR: // grow same as MEM_TRAILING
        case MEM_TRAILING:
    
            // Allocate only exactly what is needed (plus null terminator)
            if (new_bytes == SIZE_MAX) return -1;
            return resize_capacity(s, new_bytes + 1);

        case MEM_DOUBLE:

            // Double capacity until it fits
            size_t new_cap = s->cap;
            if (new_cap == 0) new_cap = 1;
            while (new_cap <= new_bytes) {
                if (new_cap > SIZE_MAX / 2) { // size_t overflow check
                    new_cap = SIZE_MAX;
                    break;
                }
                new_cap *= 2;
            }
            return resize_capacity(s, new_cap);

        case MEM_FIXED:
            // Print error and return -1 if out of fixed bounds
            if (s->cap <= new_bytes) {
                fprintf(stderr, "STRING ERROR: new text of %zu bytes exceeds FIXED cap of %zu bytes\n", new_bytes, s->cap);
                return -1;
            }
            // else do nothing
            return 0;

        default:
            fprintf(stderr, "STRING ERROR: invalid memory allocation procedure in grow_capacity()\n");
            return -1;
    }
}


static int shrink_capacity(
    String *s
) {
    switch (s->allocation_procedure) {
        case MEM_LINEAR:

            // Allocate exactly what is needed (plus null terminator)
            if (s->bytes == SIZE_MAX) return -1;
            return resize_capacity(s, s->bytes + 1);

        case MEM_TRAILING: // same as MEM_FIXED
        case MEM_FIXED:
    
            // don't shrink, do nothing
            return 0;

        case MEM_DOUBLE:

            // Halve capacity just before its smaller than bytes
            size_t new_cap = s->cap;
            while (new_cap / 2 > s->bytes)
                new_cap /= 2;
            if (new_cap == s->cap) return 0;
            return resize_capacity(s, new_cap);

        default:
            fprintf(stderr, "STRING ERROR: invalid memory allocation procedure in grow_capacity()\n");
            return -1;
    }
}


void _str_free(
    String ***list,
    size_t count
) {

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


String *str_clone(
    const String *s
) {

    // Return NULL if s is invalid
    if (s == NULL || s->text == NULL) return NULL;

    String *clone = malloc(sizeof(String));
    if (!clone) return NULL;

    size_t cap = 2 * s->bytes + 1; // match str() constructor sizing
    clone->text = malloc(cap);
    if (!clone->text) { free(clone); return NULL; }

    memcpy(clone->text, s->text, s->bytes);
    clone->text[s->bytes] = '\0';
    clone->bytes = s->bytes;
    clone->len   = s->len;
    clone->cap   = cap;
    return clone;
}


void str_info(
    const String *s,
    char *out
) {
    if (!s) return;

    // copy to result to out buffer if out != NULL,
    // else just print it to the console
    const char *text_repr = s->text ? s->text : "NULL";
    size_t total_size = sizeof(*s) + s->cap;
    if (out) {
        fmt(out, sizeof(out),
            "text=\"%s\", len=%zu, bytes=%zu, cap=%zu, String struct size=%zu, total size=%zu bytes",
            s->text ? s->text : "NULL",
            s->len,
            s->bytes,
            s->cap,
            sizeof(*s),
            total_size
        );
    } else {
        printf(
            "text=\"%s\", len=%zu, bytes=%zu, cap=%zu, String struct size=%zu, total size=%zu bytes\n",
            text_repr,
            s->len,
            s->bytes,
            s->cap,
            sizeof(*s),
            total_size
        );
        fflush(stdout);  // print immediately
    }
}


////////////////////////////// Mutation Functions /////////////////////////


int str_append(
    String *s,
    const char *suffix
) {
    if (s == NULL || s->text == NULL || suffix == NULL)
        return -1;

    // Normalize the prefix (NFC)
    // This ensures combining codepoints become canonical before insertion.
    const char *normalized = normalize_utf8_str(suffix);
    if (!normalized)
        return -1;

    size_t suffix_bytes = strlen(normalized);

    // Resize dynamic array if needed
    size_t needed = s->bytes + suffix_bytes + 1;
    if (needed > s->cap) {
        int rc = grow_capacity(s, needed);
        if (rc != 0)
            return -1;
    }

    // Append suffix characters to s->text
    memcpy(s->text + s->bytes, normalized, suffix_bytes);
    s->bytes += suffix_bytes;
    s->text[s->bytes] = '\0';

    // Update rune count
    s->len += count_utf8_runes(normalized);

    return 0;
}


int str_prepend(
    const char *prefix,
    String *s
) {
    if (!s || !s->text || !prefix)
        return -1;

    // Normalize the prefix (NFC)
    // This ensures combining codepoints become canonical before insertion.
    const char *normalized = normalize_utf8_str(prefix);
    if (!normalized)
        return -1;

    size_t prefix_bytes = strlen(normalized);

    // Ensure capacity
    size_t needed = s->bytes + prefix_bytes + 1;
    if (needed > s->cap) {
        int rc = grow_capacity(s, needed);
        if (rc != 0)
            return -1;
    }

    // Shift existing bytes to the right (including null terminator)
    memmove(
        s->text + prefix_bytes,   // destination
        s->text,                  // source
        s->bytes + 1              // +1 for '\0'
    );

    // Insert normalized prefix at the start
    memcpy(s->text, normalized, prefix_bytes);

    // Update metadata
    s->bytes += prefix_bytes;
    s->len   += count_utf8_runes(normalized);

    return 0;
}


int _str_concat(
    String **s_list,
    const size_t count,
    const ConcatOptions *opts
) {
    if (count == 0 || s_list == NULL || s_list[0] == NULL)
        return -1;

    // Default values if opts is NULL (though the macro always provides one)
    size_t output_index = (opts != NULL) ? opts->output_index : 0;

    if (output_index >= count) return -1; // invalid output index

    // Compute new total byte and rune lengths including separator
    String *sep        = (opts != NULL) ? opts->sep : NULL;
    size_t sep_bytes   = (sep && sep->text) ? sep->bytes : 0;
    size_t sep_runes   = (sep && sep->text) ? sep->len : 0;
    size_t total_bytes = 0;
    size_t total_runes = 0;
    bool first = true;
    for (size_t i = 0; i < count; i++) {
        if (!s_list[i]) continue;
        if (!first && sep != NULL) {
            total_bytes += sep_bytes;
            total_runes += sep_runes;
        }
        total_bytes += s_list[i]->bytes;
        total_runes += s_list[i]->len;
        first = false;
    }

    // Build the result in s_list[output_index]
    // Snapshot result text and bytes, then grow the result string's capacity if needed
    String *result = s_list[output_index];
    char *tmp = NULL;
    size_t tmp_bytes = result->bytes;
    if (tmp_bytes > 0) {
        tmp = malloc(tmp_bytes + 1);
        if (!tmp) return -1;
        memcpy(tmp, result->text, tmp_bytes + 1);
    }
    if (total_bytes + 1 > result->cap) {
        if (grow_capacity(result, total_bytes + 1) != 0) {
            free(tmp);
            return -1;
        }
    }

    // Append all other Strings to String at output_index
    char *dst = result->text;
    size_t offset = 0;
    bool first_written = true;
    for (size_t i = 0; i < count; i++) {
        if (!s_list[i]) continue;

        // Append separator if:
        if (
            sep_bytes > 0 &&   // the separator isn't NULL or an empty String
            !first_written      // this isnt the first text appended
        ) {
            memcpy(dst + offset, sep->text, sep_bytes);
            offset += sep_bytes;
        }

        // if the current string is the string at output_index:
        if (i == output_index) {
            // copy tmp into result->text (aka dst)
            if (tmp) {
                memcpy(dst + offset, tmp, tmp_bytes);
                offset += tmp_bytes;
                free(tmp);
                tmp = NULL;
            }
            // else tmp_bytes == 0, nothing to copy

        } else {
            // the current string is NOT at output_index
            // copy it into result->text
            memcpy(dst + offset, s_list[i]->text, s_list[i]->bytes);
            offset += s_list[i]->bytes;
        }

        // flag all subsequent text appended as not the first
        first_written = false;
    }

    dst[offset] = '\0';
    result->bytes = offset;
    result->len = total_runes;

    return 0;
}


static int str_to_case(
    String *s,
    utf8proc_int32_t (*convert)(utf8proc_int32_t)
) {
    if (s == NULL || s->text == NULL) return -1;

    size_t cap = s->bytes * 2 + 1;
    char *buf = malloc(cap);
    if (!buf) return -1;

    // Iterate rune by rune, applying toupper/tolower to each codepoint,
    // then re-encode back to UTF-8 in place.
    // Note: toupper/tolower can expand byte count (e.g. ß→SS), so we
    // write into a fresh buffer to be safe.
    const utf8proc_uint8_t *src = (const utf8proc_uint8_t *)s->text;
    utf8proc_ssize_t remaining = (utf8proc_ssize_t)s->bytes;
    size_t out = 0;
    size_t runes = 0;
    while (remaining > 0) {
        utf8proc_int32_t codepoint;
        utf8proc_ssize_t n = utf8proc_iterate(src, remaining, &codepoint);
        if (n <= 0) break;
        utf8proc_int32_t converted = convert(codepoint);
        if (out + 4 >= cap) {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp) { free(buf); return -1; }
            buf = tmp;
        }
        utf8proc_ssize_t written = utf8proc_encode_char(converted, (utf8proc_uint8_t *)buf + out);
        out += written;
        runes++;
        src += n;
        remaining -= n;
    }

    buf[out] = '\0';
    free(s->text);
    s->text  = buf;
    s->bytes = out;
    s->len   = runes;
    s->cap   = cap;
    return 0;
}


int str_to_upper(
    String *s
) {
    return str_to_case(s, utf8proc_toupper);
}


int str_to_lower(
    String *s
) {
    return str_to_case(s, utf8proc_tolower);
}


int str_insert(
    String *s,
    const String *substr,
    size_t rune_index
) {
    if (!s || !s->text || !substr || !substr->text)
        return -1;

    // Ensure rune_index is valid (allow insert at end)
    if (rune_index > s->len)
        return -1;

    // Calculate new byte length and rune length
    size_t new_bytes = s->bytes + substr->bytes;
    size_t new_runes = s->len + substr->len;

    // Resize if needed
    if (new_bytes + 1 > s->cap) {
        int rc = grow_capacity(s, new_bytes + 1);
        if (rc != 0) return -1;
    }

    // Find the byte_index of the requested rune_index
    size_t byte_index = rune_to_byte_index(s, rune_index);

    // Shift existing content right
    memmove(s->text + byte_index + substr->bytes, s->text + byte_index, s->bytes - byte_index + 1); // +1 to move null terminator

    // Copy substring into place
    memcpy(s->text + byte_index, substr->text, substr->bytes);

    s->bytes = new_bytes;
    s->len = new_runes;
    return 0;
}


int str_replace(
    String *s,
    const String *old_sub,
    const String *new_sub,
    const char *mode
) {
    if (!s || !s->text || !old_sub || !old_sub->text || !new_sub || !new_sub->text)
        return -1;

    if (old_sub->bytes == 0 || s->bytes < old_sub->bytes) return -1; // invalid

    if (!mode) mode = "first"; // default mode

    bool index_found = false;
    size_t pos = 0; // byte offset of match, only valid when index_found == true
    size_t i; // i = current index in String text iteration

    // Find index of first/last/all substr occurrence(s) to replace
    if (strcmp(mode, "all") == 0) {

        // Count all substr occurrences
        size_t count = 0;
        for (i = 0; i <= s->bytes - old_sub->bytes; i++) {
            if (strncmp(s->text + i, old_sub->text, old_sub->bytes) == 0) {
                count++;
                i += old_sub->bytes - 1; // move past this occurrence
            }
        }
        if (count == 0) return 0; // nothing to replace

        // Update memory capacity if needed
        size_t new_bytes = (size_t)((ssize_t)s->bytes + (ssize_t)count * \
            ((ssize_t)new_sub->bytes - (ssize_t)old_sub->bytes));
        // NOTE: Cast to ssize_t temporarily to avoid size_t underflow:
        // if old_sub->bytes > new_sub->bytes, the subtraction would wrap
        // around to a huge positive number instead of going negative.
        if (new_bytes + 1 > s->cap) {
            if (grow_capacity(s, new_bytes + 1) != 0)
                return -1;
        }

        // Rebuild string with replacements
        char *buf = malloc(new_bytes + 1);
        if (!buf) return -1;
        i = 0;
        size_t j = 0;
        while (i < s->bytes) {
            if (i <= s->bytes - old_sub->bytes &&
                strncmp(s->text + i, old_sub->text, old_sub->bytes) == 0) {
                memcpy(buf + j, new_sub->text, new_sub->bytes);
                i += old_sub->bytes;
                j += new_sub->bytes;
            } else {
                buf[j++] = s->text[i++];
            }
        }
        buf[j] = '\0';

        // Update s with new buffer
        free(s->text);
        s->text = buf;
        s->bytes = j;
        s->len = (size_t)((ssize_t)s->len +
            (ssize_t)count * ((ssize_t)new_sub->len - (ssize_t)old_sub->len));
        // NOTE: Same underflow risk for rune counts — cast to ssize_t
        // so the subtraction can go negative before being scaled by count.
        
        // Shrink memory capacity if needed
        if (shrink_capacity(s) != 0)
            return -1;
        return 0;

    } else if (strcmp(mode, "first") == 0) {
        for (i = 0; i <= s->bytes - old_sub->bytes; i++) {
            if (strncmp(s->text + i, old_sub->text, old_sub->bytes) == 0) {
                pos = i;
                index_found = true;
                break;
            }
        }

    } else if (strcmp(mode, "last") == 0) {
        for (i = s->bytes - old_sub->bytes + 1; i-- > 0;) {
            if (strncmp(s->text + i, old_sub->text, old_sub->bytes) == 0) {
                pos = i;
                index_found = true;
                break;
            }
        }

    } else {
        return -1; // invalid mode
    }

    // Single replacement ("first" or "last")
    if (!index_found) return 0; // substr not found, nothing to replace

    // Update memory capacity if needed
    size_t new_bytes = (size_t)((ssize_t)s->bytes + 
        (ssize_t)new_sub->bytes - (ssize_t)old_sub->bytes);
    if (new_bytes + 1 > s->cap) {
        if (grow_capacity(s, new_bytes + 1) != 0)
            return -1;
    }

    // Shift remainder to make space / remove old_sub
    if (new_sub->bytes != old_sub->bytes) {
        memmove(s->text + pos + new_sub->bytes,
                s->text + pos + old_sub->bytes,
                s->bytes - pos - old_sub->bytes + 1); // include null terminator
    }

    // Copy in new_sub
    memcpy(s->text + pos, new_sub->text, new_sub->bytes);
    s->bytes = new_bytes;
    s->len = (size_t)((ssize_t)s->len +
        (ssize_t)new_sub->len - (ssize_t)old_sub->len);
    // NOTE: Cast to ssize_t temporarily to avoid size_t underflow:
    // if old_sub->len > new_sub->len, the subtraction would wrap
    // around to a huge positive number instead of going negative.

    // Shrink memory capacity if needed
    if (shrink_capacity(s) != 0)
        return -1;
    return 0;
}


int _str_repeat(
    String *s,
    const size_t n,
    RepeatOptions *opts
) {

    // validate s
    if (!s || !s->text || !opts) return -1;

    // output to text buffer if text_buffer != NULL,
    // else modify s in place
    if (opts->text_buffer) {

        // buffer_size is required when text_buffer is provided
        if (opts->buffer_size == (size_t)-1) {
            fprintf(stderr, "str_repeat() error: text_buffer provided but buffer_size not set. "
                            "Pass .buffer_size=sizeof(your_buffer).\n");
            return -1;
        }

        // write empty string if n == 0
        if (n == 0) { opts->text_buffer[0] = '\0'; return 0; }

        // check for overflow before multiplying
        if (s->bytes != 0 && n > SIZE_MAX / s->bytes) return -1;

        // check buffer is large enough to hold repeated string
        size_t needed = s->bytes * n + 1; // +1 for null terminator
        if (needed > opts->buffer_size) {
            fprintf(stderr, "str_repeat() error: buffer_size=%zu is too small, need %zu bytes.\n",
                            opts->buffer_size, needed);
            return -1;
        }

        // copy s->text into text_buffer n times
        for (size_t i = 0; i < n; i++)
            memcpy(opts->text_buffer + i * s->bytes, s->text, s->bytes);
        opts->text_buffer[s->bytes * n] = '\0';

    } else {

        // clear string if n == 0
        if (n == 0) return str_clear(s);

        // check for overflow before multiplying
        if (s->bytes != 0 && n > SIZE_MAX / s->bytes) return -1;

        // grow allocation if needed
        size_t old_bytes = s->bytes;
        size_t new_bytes = old_bytes * n;
        size_t needed    = new_bytes + 1; // +1 for null terminator
        if (needed > s->cap) {
            int rc = grow_capacity(s, needed);
            if (rc != 0) return -1;
        }

        // copy s->text repeatedly into itself, starting after the first copy
        for (size_t i = 1; i < n; i++)
            memcpy(s->text + i * old_bytes, s->text, old_bytes);
        s->text[new_bytes] = '\0';

        // update rune and byte count
        s->bytes = new_bytes;
        s->len   = s->len * n;
    }
    return 0;
}


int str_remove(
    String *s,
    size_t start,
    size_t len
) {
    if (!s || !s->text) return -1;
    if (start >= s->len) return 0;
    if (start + len > s->len)
        len = s->len - start;

    size_t byte_start, byte_len;
    rune_range_to_byte_range(s->text, start, len, &byte_start, &byte_len);

    memmove(s->text + byte_start,
            s->text + byte_start + byte_len,
            s->bytes - byte_start - byte_len + 1); // +1 for null terminator
    s->bytes -= byte_len;
    s->len   -= len;

    // Shrink memory capacity if needed
    if (shrink_capacity(s) != 0)
        return -1;
    return 0;
}


int str_trim(
    String *s
) {
    if (!s || !s->text) return -1;
    str_trim_left(s);
    str_trim_right(s);
    return 0;
}


int str_trim_left(
    String *s
) {
    if (!s || !s->text) return -1;

    // find first non-whitespace rune
    size_t start_byte = 0;
    utf8proc_int32_t codepoint;
    const utf8proc_uint8_t *p = (const utf8proc_uint8_t *)s->text;
    ssize_t remaining = (ssize_t)s->bytes;
    size_t whitespace_runes = 0;
    while (remaining > 0) {
        ssize_t n = utf8proc_iterate(p, remaining, &codepoint);
        if (n <= 0) break;
        if (!is_whitespace_rune(codepoint)) break;
        start_byte += n;
        p += n;
        remaining -= n;
        whitespace_runes++;
    }

    // Remove left side whitespace
    if (start_byte > 0) {
        memmove(s->text, s->text + start_byte, s->bytes - start_byte + 1);
        s->bytes -= start_byte;
        s->len -= whitespace_runes;
    }
    return 0;
}


int str_trim_right(
    String *s
) {
    if (!s || !s->text) return -1;
    if (s->bytes == 0) return 0;

    const utf8proc_uint8_t *p = (const utf8proc_uint8_t *)s->text;
    utf8proc_ssize_t remaining = (utf8proc_ssize_t)s->bytes;
    size_t last_non_ws_byte = 0;
    size_t last_non_ws_rune = 0;
    size_t current_byte = 0;
    size_t current_rune = 0;

    while (remaining > 0) {
        utf8proc_int32_t codepoint;
        utf8proc_ssize_t n = utf8proc_iterate(p, remaining, &codepoint);
        if (n <= 0) break;
        if (!is_whitespace_rune(codepoint)) {
            last_non_ws_byte = current_byte + n;
            last_non_ws_rune = current_rune + 1;
        }
        p += n;
        current_byte += n;
        current_rune++;
        remaining -= n;
    }

    s->text[last_non_ws_byte] = '\0';
    s->bytes = last_non_ws_byte;
    s->len   = last_non_ws_rune;
    return 0;
}


int str_clear(
    String *s
) {
    if (s == NULL || s->text == NULL)
        return -1;

    // Set text to empty string
    s->text[0] = '\0';
    s->len = 0;
    s->bytes = 0;

    // Shrink memory capacity
    if (shrink_capacity(s) != 0)
        return -1;
    return 0;
}


int str_overwrite(
    String *s,
    const char *new_text
) {
    if (s == NULL || s->text == NULL || new_text == NULL)
        return -1;

    // Normalize the new text to NFC form
    // NOTE: the text normalize_utf8_str() returns is null terminated
    char *normalized = normalize_utf8_str(new_text);
    if (!normalized) return -1;

    // Get the byte and rune lengths of the normalized string
    size_t new_bytes = strlen(normalized);
    size_t new_len = count_utf8_runes(normalized);

    // Resize if needed
    size_t needed = new_bytes + 1;
    if (needed > s->cap) {
        if (grow_capacity(s, needed) != 0) {
            free(normalized);
            return -1;
        }
    }

    // Copy the new text
    memcpy(s->text, normalized, new_bytes + 1);
    free(normalized);

    // Update string metadata
    s->bytes = new_bytes;
    s->len = new_len;

    // Shrink if appropriate
    if (shrink_capacity(s) != 0)
        return -1;

    return 0;
}


////////////////////////////// Query Functions ////////////////////////////


bool str_equals(
    const String *a,
    const String *b
) {
    if (!a || !b) return false;

    // Quick rune length check
    if (a->len != b->len) return false;

    // If both empty, they are equal
    if (a->len == 0) return true;

    // Quick bytes length check
    if (a->bytes != b->bytes) return false;

    // Compare raw bytes (faster than strcmp because we know length)
    return memcmp(a->text, b->text, a->bytes) == 0;
}


bool str_is_empty(
    const String *s
) {
    if (s == NULL) return true; // treat NULL as empty
    return s->len == 0;
}


bool str_starts_with(
    const String *s,
    const String *prefix
) {
    if (s == NULL || prefix == NULL) return false;
    if (prefix->bytes > s->bytes) return false;
    return memcmp(s->text, prefix->text, prefix->bytes) == 0;
}


bool str_ends_with(
    const String *s,
    const String *suffix
) {
    if (s == NULL || suffix == NULL) return false;
    if (suffix->bytes > s->bytes) return false;
    return memcmp(s->text + s->bytes - suffix->bytes, suffix->text, suffix->bytes) == 0;
}


bool str_contains(
    const String *s,
    const String *substr) {
    return str_index_of(s, substr, "first") != -1;
}


size_t str_count(
    const String *s,
    const String *substr
) {
    if (!s || !substr || !s->text || !substr->text) return 0;
    if (substr->bytes == 0 || s->bytes < substr->bytes) return 0;
    size_t count = 0;
    size_t n = s->bytes;
    size_t m = substr->bytes;
    for (size_t i = 0; i <= n - m; ) {
        if (memcmp(s->text + i, substr->text, m) == 0) {
            count++;
            i += m;
        } else {
            utf8proc_int32_t codepoint;
            utf8proc_ssize_t n_bytes = utf8proc_iterate(
                (const utf8proc_uint8_t *)s->text + i,
                (utf8proc_ssize_t)(n - i),
                &codepoint
            );
            i += (size_t)n_bytes;
        }
    }
    return count;
}


size_t str_index_of(
    const String *s,
    const String *substr,
    const char *mode
) {

    // Validate args
    if (!s || !s->text || !substr || !substr->text) return -1;
    if (s->bytes < substr->bytes) return -1;

    // Set default mode to "first"
    if (!mode) mode = "first";

    // Empty substring is always found at index 0 (first) or last rune index (last)
    if (substr->bytes == 0) {
        if (strcmp(mode, "last") == 0) return s->len;
        return 0;
    }

    // Search by bytes - only convert to rune index on a match
    const char *text = s->text;
    const char *pat  = substr->text;
    size_t n = s->bytes;
    size_t m = substr->bytes;
    if (strcmp(mode, "first") == 0) {
        for (size_t i = 0; i <= n - m; i++) {
            if (memcmp(text + i, pat, m) == 0)
                return byte_to_rune_index(s, i);
        }
    } else if (strcmp(mode, "last") == 0) {
        for (size_t i = n - m; ; i--) {
            if (memcmp(text + i, pat, m) == 0)
                return byte_to_rune_index(s, i);
            if (i == 0) break;
        }
    } else {
        return -1; // invalid mode
    }

    return -1; // not found
}


size_t* str_indices_of(
    const String *s,
    const String *substr,
    size_t *count
) {

    // Return NULL for invalid arg inputs
    if (!s || !s->text || !substr || !substr->text || !count)
        return NULL;

    // Set count to 0 and return NULL to represent empty list if s is empty
    *count = 0;
    if (substr->bytes == 0 || s->bytes < substr->bytes)
        return NULL;

    // Allocate worst-case array
    size_t *indices = malloc(s->len * sizeof(size_t));
    if (!indices) return NULL;

    // fill worst-case sized array with each substr match in s
    const char *text = s->text;
    size_t n = s->bytes;
    size_t m = substr->bytes;
    for (size_t i = 0; i <= n - m; ) {
        if (text[i] == substr->text[0] && memcmp(text + i, substr->text, m) == 0) {
            // Convert byte offset to rune index before storing
            indices[(*count)++] = byte_to_rune_index(s, i);
            i += m; // skip past this match (in bytes)
        } else {
            // Advance by one rune (not one byte) to avoid landing mid-rune
            utf8proc_int32_t codepoint;
            utf8proc_ssize_t n_bytes = utf8proc_iterate(
                (const utf8proc_uint8_t *)text + i,
                (utf8proc_ssize_t)(n - i),
                &codepoint
            );
            i += (size_t)n_bytes;
        }
    }

    // Trim array to exact size (O(1) for shrinking)
    if (*count == 0) {
        free(indices);
        return NULL;
    }
    size_t *result = realloc(indices, *count * sizeof(size_t));
    if (!result)
        return indices;
    return result;
}


////////////////////////////// Extract Functions //////////////////////////


String **str_split(
    const String *s,
    const String *delim,
    size_t *out_count
) {
    if (!s || !s->text || !delim || !delim->text || !out_count) return NULL;

    *out_count = 0;
    size_t n = s->bytes;
    size_t m = delim->bytes;
    if (n == 0 || m == 0) return NULL;

    // Allocate Worst case: every rune is a delimiter
    String **result = malloc((s->len + 1) * sizeof(String *));
    if (!result) return NULL;

    size_t byte_pos   = 0;
    size_t rune_pos   = 0;
    size_t part_start_byte = 0;
    size_t part_start_rune = 0;

    while (byte_pos <= n - m) {
        if (memcmp(s->text + byte_pos, delim->text, m) == 0) {
            // Emit part from part_start to here
            size_t part_bytes = byte_pos - part_start_byte;
            size_t part_runes = rune_pos - part_start_rune;

            String *part = malloc(sizeof(String));
            if (!part) goto cleanup;
            part->text = malloc(part_bytes + 1);
            if (!part->text) { free(part); goto cleanup; }

            memcpy(part->text, s->text + part_start_byte, part_bytes);
            part->text[part_bytes] = '\0';
            part->bytes = part_bytes;
            part->len   = part_runes;
            part->cap   = part_bytes + 1;
            result[(*out_count)++] = part;

            // Skip delimiter
            byte_pos += m;
            rune_pos += delim->len;
            part_start_byte = byte_pos;
            part_start_rune = rune_pos;
        } else {
            utf8proc_int32_t codepoint;
            utf8proc_ssize_t n_bytes = utf8proc_iterate(
                (const utf8proc_uint8_t *)s->text + byte_pos,
                (utf8proc_ssize_t)(n - byte_pos),
                &codepoint
            );
            byte_pos += (size_t)n_bytes;
            rune_pos++;
        }
    }

    // Emit final part (everything after last delimiter)
    size_t part_bytes = n - part_start_byte;
    size_t part_runes = s->len - part_start_rune;
    String *part = malloc(sizeof(String));
    if (!part) goto cleanup;
    part->text = malloc(part_bytes + 1);
    if (!part->text) { free(part); goto cleanup; }
    memcpy(part->text, s->text + part_start_byte, part_bytes);
    part->text[part_bytes] = '\0';
    part->bytes = part_bytes;
    part->len   = part_runes;
    part->cap   = part_bytes + 1;
    result[(*out_count)++] = part;

    return result;

    cleanup:
    for (size_t j = 0; j < *out_count; j++)
        str_free(&result[j]);
    free(result);
    return NULL;
}


int _str_slice(
    String *s,
    ssize_t start,
    ssize_t end,
    SliceOptions *opts
) {

    // validate s
    if (!s || !s->text || !opts) return -1;

    // empty string edge case (avoid divide by zero in modulus below)
    if (s->len == 0) {
        if (opts->text_buffer) {
            opts->text_buffer[0] = '\0';
        } // else do nothing
        return 0;
    }
    ssize_t len = (ssize_t)s->len;

    // Negative indices wrap around, and
    // indeces longer than s->len are wrapped.
    // DETAILS: inner modulus brings it into [-len, len] range,
    // then +len shifts negatives positive,
    // then the outer % len cleans up the cases that were already positive.
    start = ((start % len) + len) % len;
    end   = ((end   % len) + len) % len;

    // end==0 after wrapping means it was a multiple of len (e.g. len itself),
    // which is valid as the exclusive upper bound — restore it to len
    if (end == 0) end = len;
    if (start > end) return -1;

    // get byte range from rune range
    size_t byte_start, byte_len;
    rune_range_to_byte_range(s->text, start, end - start, &byte_start, &byte_len);

    // output to text buffer if text_buffer != NULL,
    // else modify s in place
    if (opts->text_buffer) {

        // buffer_size is required when text_buffer is provided
        if (opts->buffer_size == (size_t)-1) {
            fprintf(stderr, "str_slice() error: text_buffer provided but buffer_size not set. "
                            "Pass .buffer_size=sizeof(your_buffer).\n");
            return -1;
        }

        // check buffer is large enough to hold slice
        size_t needed = byte_len + 1; // +1 for null terminator
        if (needed > opts->buffer_size) {
            fprintf(stderr, "str_slice() error: buffer_size=%zu is too small, need %zu bytes.\n",
                            opts->buffer_size, needed);
            return -1;
        }

        // copy slice into text_buffer
        memcpy(opts->text_buffer, s->text + byte_start, byte_len);
        opts->text_buffer[byte_len] = '\0';
        return 0;

    } else {

        // copy slice into buffer to overwrite s with
        char *buffer = malloc(byte_len + 1);
        if (!buffer) return -1;
        memcpy(buffer, s->text + byte_start, byte_len);
        buffer[byte_len] = '\0';
        int rc = str_overwrite(s, buffer);
        free(buffer);
        return rc;
    }
}


////////////////////////////// Char Array Formatting //////////////////////


char *fmt(
    char *buf,
    const size_t cap,
    const char *fmt_text,
    ...
) {
    if (!buf || cap == 0 || !fmt_text) return NULL;

    va_list args;
    va_start(args, fmt_text);
    int n = vsnprintf(buf, cap, fmt_text, args);
    va_end(args);

    if (n < 0) return NULL;  // encoding/formatting error
    return buf;
}


size_t fmt_append(
    char *buf,
    const size_t cap,
    const char *fmt_text,
    ...
) {
    /// Append fmt_text (plus formatting) to buffer ///

    // validate buffer and fmt_text args
    if (!buf || !fmt_text || cap == 0) return (size_t)-1;
    size_t pos = 0; for (; pos < cap && buf[pos]; pos++) {} // strlen() but safe for invalid strings without '\0'
    if (pos == cap) return (size_t)-1;

    va_list args;
    va_start(args, fmt_text);
    int n = vsnprintf(buf + pos, cap - pos, fmt_text, args);
    // NOTE: vsnprintf() returns:
    // On success:
    // Returns the number of characters that would have been written (excluding the null terminator). If the output was truncated due to insufficient space, it still returns the number of characters that would have been written if there had been enough space. It tells you how much space you would have needed for the full formatted string.
    // On failure:
    // Returns a negative value if an encoding error occurs.
    va_end(args);

    // snprintf error, leave pos unchanged
    if (n < 0) return (size_t)-1;

    return (size_t)n; // return would-be length, so caller can detect truncation
}

///////////////////////////////////////////////////////////////////////////

