#include "string_util.h"
#include <stdio.h>

int main(void) {

    // init string
    String *s1 = str("Hello");
    char buffer[1024]; str_info(s1, buffer, sizeof(buffer));
    printf("%s\n", buffer);
    // outputs: text="Hello", len=5, bytes=5, cap=11, String struct size=32, total size=43 bytes

    // append char array to string
    str_append(s1, ", world");
    buffer[0] = '\0'; str_info(s1, buffer, sizeof(buffer));
    printf("%s\n", buffer);
    // outputs: text="Hello, world", len=12, bytes=12, cap=22, String struct size=32, total size=54 bytes

    // prepend char array to string
    str_prepend("... ", s1);
    buffer[0] = '\0'; str_info(s1, buffer, sizeof(buffer));
    printf("%s\n", buffer);
    // outputs: text="... Hello, world", len=16, bytes=16, cap=22, String struct size=32, total size=54 bytes

    // free string struct and text
    str_free(&s1);

    // concat multiple strings into one, with option to set which string
    // to store the output in (defaults to first list item)
    String *a = str("Hello"), *b = str("東京"), *c = str("¡café!naïve"), *d = str("😀🍓🌍❌✅");
    String *parts[] = {a, b, c, d};
    str_concat(parts, .output_index=2);
    printf("%s\n", c->text); // "Hello東京¡café!naïve😀🍓🌍❌✅"
    String *e = str("→↙●■▲");
    String *f = str("∞∑√");
    String *g = str("★♥🔒🔓");
    str_concat(((String *[]){c, e, f, g}));
    printf("%s\n", c->text); // "Hello東京¡café!naïve😀🍓🌍❌✅→↙●■▲∞∑√★♥🔒🔓"

    // free multiple strings at once
    str_free(&a, &b, &c, &d, &e, &f, &g);

    return 0;
}
