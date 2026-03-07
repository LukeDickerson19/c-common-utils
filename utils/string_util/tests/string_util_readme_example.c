#include "string_util.h"
#include <stdio.h>

int main(void) {

    // init string
    String *s1 = str("Hello");
    printf("text = \"%s\", length = %d, memory allocated = %d bytes\n", s1->text, s1->len, s1->cap);
    // outputs: "text = "Hello", length = 5, memory allocated = 11 bytes"

    // append char array to string
    str_append(s1, ", world");
    printf("text = \"%s\", length = %d, memory allocated = %d bytes\n", s1->text, s1->len, s1->cap);
    // outputs: text = "Hello, world", length = 12, memory allocated = 22 byte

    // prepend char array to string
    str_prepend("... ", s1);
    printf("text = \"%s\", length = %d, memory allocated = %d bytes\n", s1->text, s1->len, s1->cap);
    // outputs: text = "... Hello, world", length = 16, memory allocated = 22 bytes

    // free string struct and text
    str_free(&s1);

    // concat multiple strings into one, with options to set which string
    // to store the output in (defaults to first list item), and whether
    // to free the others or not (defaults to false)
    String *a = str("AAA"), *b = str("BBB"), *c = str("CCC"), *d = str("DDD");
    String *parts[] = {a, b, c, d};
    str_concat(parts, .output_index=2);
    printf("%s\n", c->text); // "AAABBBCCCDDD"
    String *e = str("EEE");
    String *f = str("FFF");
    String *g = str("GGG");
    str_concat(((String *[]){c, e, f, g}));
    printf("%s\n", c->text); // "AAABBBCCCDDDEEEFFFGGG"

    // free multiple strings at once
    str_free(&a, &b, &c, &d, &e, &f, &g);

    return 0;
}
