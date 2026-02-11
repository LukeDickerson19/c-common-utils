

#include "string_utils.h"
#include <stdlib.h>     // malloc, realloc, free
#include <string.h>     // strlen, strcpy, strcat, memcpy, memmove
#include <stdbool.h> // for bool



String str(const char *text) {
    size_t len = strlen(text);
    size_t cap = 2 * len + 1; // +1 for null terminator
    char *content = malloc(cap * sizeof(char));
    if (content == NULL) {
        exit(1); // Handle allocation failure (could be improved)
    }
    strcpy(content, text);
    return (String){
        .text = content,
        .len  = len,
        .cap  = cap
    };
}

int _append(String *dst, String *suffix, const AppendOptions *opts) {
    if (dst == NULL || dst->text == NULL || suffix == NULL || suffix->text == NULL)
        return -1;

    // Default if opts is NULL (though macro always supplies one)
    bool free_suffix = (opts != NULL) ? opts->free_suffix : false;

    // Resize dynamic array if needed
    size_t needed = dst->len + suffix->len + 1;
    if (needed > dst->cap) {
        size_t new_cap = dst->cap;
        while (needed > new_cap)
            new_cap *= 2;
        char *new_content = realloc(dst->text, new_cap);
        if (new_content == NULL)
            return -1;
        dst->text = new_content;
        dst->cap  = new_cap;
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
    size_t needed = dst->len + prefix->len + 1;
    if (needed > dst->cap) {
        size_t new_cap = dst->cap;
        while (needed > new_cap)
            new_cap *= 2;
        char *new_content = realloc(dst->text, new_cap);
        if (new_content == NULL)
            return -1;
        dst->text = new_content;
        dst->cap  = new_cap;
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

int _concat(String *str_lst[], const ConcatOptions *opts) {
    if (str_lst == NULL || str_lst[0] == NULL)
        return -1;

    // Default values if opts is NULL (though the macro always provides one)
    int  output_index = (opts != NULL) ? opts->output_index : 0;
    bool free_others  = (opts != NULL) ? opts->free_others  : false;

    // Validate output_index
    int num_strings = 0;
    while (str_lst[num_strings] != NULL)
        num_strings++;

    if (output_index < 0 || output_index >= num_strings)
        return -1;  // invalid output index

    // build the result in str_lst[output_index]
    String *result = str_lst[output_index];

    // compute total length
    size_t total_len = 0;
    for (int i = 0; i < num_strings; i++)
        total_len += str_lst[i]->len;

    // resize result string if necessary
    if (total_len + 1 > result->cap) {
        size_t new_cap = result->cap;
        while (total_len + 1 > new_cap)
            new_cap *= 2;
        char *new_ptr = realloc(result->text, new_cap);
        if (new_ptr == NULL)
            return -1;
        result->text = new_ptr;
        result->cap = new_cap;
    }

    // If output_index != 0, move str_lst[0] content to output_index first
    // (only if we're not already building in the first slot)
    if (output_index != 0) {
        // Make sure result has enough space (already checked above)
        // Copy the original first string into the output position
        if (result->text != str_lst[0]->text) // avoid self-overlap
            memcpy(result->text, str_lst[0]->text, str_lst[0]->len + 1);
        result->len = str_lst[0]->len;
    } else {
        // Already building in place — start from len= original len
        result->len = result->len;  // redundant but clear
    }

    // Append all other strings (skip the output_index one) 
    // and free them immediately if requested
    char *dst = result->text + result->len;
    for (int i = 0; i < num_strings; i++) {
        if (i == output_index)
            continue;

        size_t len = str_lst[i]->len;
        memcpy(dst, str_lst[i]->text, len);
        dst += len;
        result->len += len;

        // Free right after copying if requested
        if (free_others) {
            free_string(str_lst[i]);
            // Note: we don't need to NULL the pointer here if you don't plan to reuse
            // the array afterward — but it's harmless and safer to do so
            str_lst[i] = NULL;  // optional but recommended to prevent misuse
        }
    }
    *dst = '\0';  // ensure null termination

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
        i++;
    }

    return 0;
}