
#ifndef STRING_UTILS_H
#define STRING_UTILS_H


typedef struct {
    char *text;
    size_t len; // length in characters
    size_t max_len;
} String;

String str(char *content, size_t max_len);

int append(String text, String suffix);

int prepend(String prefix, String text);

// void concat(String base);

int free_strings(String *string_list);

// this macro exists to simulate unlimited args in C
// #define concat(base_str, ...) _log_print((log), (msg), &(StringSubsections){ DEFAULT_PRINT_OPTIONS, ##__VA_ARGS__ })
// NOTE: __VA_ARGS__ override default print options because when they're later in the struct initialization
// The prepended "##" characters is a GNU extension that removes the comma if __VA_ARGS__ is empty. This is widely supported but not part of the C standard.



#endif


