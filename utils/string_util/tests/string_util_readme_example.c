#include "string_util.h"
#include <stdio.h>

int main(void) {

    // init string
    String *s1 = str("Hello");
    str_info(s1, NULL); // prints: text="Hello", len=5, bytes=5, cap=11, String struct size=32, total size=43 bytes

    // append char array to string
    str_append(s1, ", world");
    str_info(s1, NULL); // prints: text="Hello, world", len=12, bytes=12, cap=14, String struct size=40, total size=54 bytes

    // prepend char array to string
    str_prepend("... ", s1);
    // capture str_info() output instead of printing it by passing a non-NULL Buffer struct pointer
    char buf_text[128]; // stack char array with length known at compile time doesnt need to be freed
    Buffer buf_struct = { .text = buf_text, .cap=sizeof(buf_text), .pos = 0 }; // stack buffer doesn't need to be freed
    Buffer *buf = &buf_struct;
    str_info(s1, buf);
    printf("%s\n", buf->text); // prints: text="... Hello, world", len=16, bytes=16, cap=18, String struct size=40, total size=58 bytes

    // free string struct and text
    str_free(&s1);

    // concat multiple strings into one, with option to set which string
    // to store the output in (defaults to first list item)
    String *a = str("Hello"), *b = str("東京"), *c = str("¡café!naïve"), *d = str("😀🍓🌍❌✅");
    String *parts[] = {a, b, c, d};
    str_concat(parts, .output_index=2);
    printf("%s\n", c->text); // prints: Hello東京¡café!naïve😀🍓🌍❌✅
    String *e = str("→↙●■▲");
    String *f = str("∞∑√");
    String *g = str("★♥🔒🔓");
    str_concat(((String *[]){c, e, f, g}));
    printf("%s\n", c->text); // prints: Hello東京¡café!naïve😀🍓🌍❌✅→↙●■▲∞∑√★♥🔒🔓

    // init formatted string
    String *h = str(fmt(buf, "formatted😀🍓🌍%s", "string√★♥🔒🔓"));
    printf("%s\n", h->text); // prints: formatted😀🍓🌍string√★♥🔒🔓

    // free multiple strings at once
    str_free(&a, &b, &c, &d, &e, &f, &g, &h);

    return 0;
}
