#include "string_utils.h";
#include <stddef.h>;

String str(char *text_content, int max_char_length) {
    char *text = malloc(max_char_length * sizeof(char));
    text = text_content;
    String s = {
        text,
        strlen(text),
        max_char_length
    };
    return s;
}
String str(const char *text_content, size_t max_char_length) {
    String s = {0};
    s.text = malloc(max_char_length);
    if (!s.text) return s;
    strncpy(s.text, text_content, max_char_length - 1);
    s.text[max_char_length - 1] = '\0';
    s.len = strlen(text_content);
    s.max_len = max_char_length;
    return s;
}

int append(String text, String suffix) {
    if (text.len + suffix.len > text.max_len) {
        perror("cannot append suffix of length %d characters to a\npreallocated string with a max length of %d chararacters", suffix.len, text.max_len);
        return -1;
    }
    text.len += snprintf(
        text.text + text.len,
        text.max_len - text.len,
        "%s", suffix.text
    );
    return 0;
}

int prepend(String prefix, String text) {
    if (prefix.len + text.len > text.max_len) {
        perror("cannot prepend prefix of length %d characters to a\npreallocated string with a max length of %d chararacters", prefix.len, text.max_len);
        return -1;
    }
    text.len = snprintf(
        text.text,
        text.max_len,
        "%s%s", prefix.text, text.text
    );
    return 0;
}

int free_strings(String *string_list) {
    for 
}