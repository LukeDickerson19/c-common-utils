

#include "string_utils.h"
#include <stdlib.h>     // malloc, realloc, free
#include <string.h>     // strlen, strcpy, strcat, memcpy, memmove
#include <stdbool.h> // for bool


String str(const char *text) {
    if (text == NULL) text = "";
    size_t len = strlen(text);
    size_t cap = 2 * len + 1; // +1 for null terminator
    char *content = malloc(cap);
    if (!content) {
        exit(1);  // Handle allocation failure (could be improved)
    }
    memcpy(content, text, len);
    content[len] = '\0';                // explicit and clear
    return (String){
        .text = content,
        .len = len,
        .cap = cap
    };
}

int resize_string(String *dst, size_t new_len) {
    size_t new_cap = dst->cap;
    while (new_len + 1 > new_cap)
        new_cap *= 2;
    char *new_content = realloc(dst->text, new_cap);
    if (new_content == NULL)
        return -1;
    dst->text = new_content;
    dst->cap = new_cap;
    return 0;
}

int _append(String *dst, String *suffix, const AppendOptions *opts) {
    if (dst == NULL || dst->text == NULL || suffix == NULL || suffix->text == NULL)
        return -1;

    // Default if opts is NULL (though macro always supplies one)
    bool free_suffix = (opts != NULL) ? opts->free_suffix : false;

    // Resize dynamic array if needed
    size_t needed = dst->len + suffix->len + 1;
    if (needed > dst->cap) {
        int rc = resize_string(dst, needed);
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
        free_string(suffix);

    return 0;
}

int _prepend(String *prefix, String *dst, const PrependOptions *opts) {
    if (dst == NULL || dst->text == NULL || prefix == NULL || prefix->text == NULL)
        return -1;

    // Default if opts is NULL
    bool free_prefix = (opts != NULL) ? opts->free_prefix : false;

    // Resize if needed
    // Resize dynamic array if needed
    size_t needed = dst->len + prefix->len + 1;
    if (needed > dst->cap) {
        int rc = resize_string(dst, needed);
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
        free_string(prefix);

    return 0;
}

int _concat(String *str_lst[], const size_t count, const ConcatOptions *opts) {
    if (count <= 0 || str_lst == NULL || str_lst[0] == NULL)
        return -1;

    // Default values if opts is NULL (though the macro always provides one)
    int  output_index = (opts != NULL) ? opts->output_index : 0;
    bool free_others  = (opts != NULL) ? opts->free_others  : false;

    if (output_index < 0 || output_index >= count)
        return -1;  // invalid output index

    // build the result in str_lst[output_index]
    String *result = str_lst[output_index];

    // compute total length
    size_t total_len = 0;
    for (int i = 0; i < count; i++)
        total_len += str_lst[i]->len;

    // resize result string if necessary
    if (total_len + 1 > result->cap) {
        int rc = resize_string(result, total_len + 1);
        if (rc != 0)
            return -1;
    }

    // If output_index != 0, copyt result's text to a tmp char*
    char *tmp;
    size_t tmp_len;
    if (output_index != 0) {
        tmp_len = result->len;
        if (tmp_len > 0) {
            tmp = malloc(tmp_len + 1);
            if (!tmp) return -1;
            memcpy(tmp, result->text, tmp_len + 1);
        }
    }

    // Append all other strings
    // and free them immediately if requested
    char *dst = result->text;
    size_t offset = 0;
    for (int i = 0; i < count; i++) {
        if (i == output_index) {
            if (output_index == 0) {
                offset += result->len;
            } else if (tmp_len > 0) {
                memcpy(dst + offset, tmp, tmp_len);
                offset += tmp_len;
                free(tmp);
            }
        } else {
            size_t len = str_lst[i]->len;
            memcpy(dst + offset, str_lst[i]->text, len);
            offset += len;

            // Free right after copying if requested
            if (free_others) {
                free_string(str_lst[i]);
                // Note: we don't need to NULL the pointer here if you don't plan to reuse
                // the array afterward — but it's harmless and safer to do so
                str_lst[i] = NULL;  // optional but recommended to prevent misuse
            }
        }
    }
    dst[offset] = '\0';
    result->len = offset;

    return 0;
}

int free_string(String *string) {
    if (string == NULL) {
        return -1;
    }

    if (string->text != NULL) {
        free(string->text);
        string->text = NULL;
    }

    string->len = 0;
    string->cap = 0;

    return 0;
}

int free_strings(String *string_list[]) {
    if (string_list == NULL) {
        return -1;
    }

    int i = 0;
    while (string_list[i] != NULL) {
        free_string(string_list[i]);
        string_list[i] = NULL;  // optional but recommended to prevent misuse
        i++;
    }

    return 0;
}