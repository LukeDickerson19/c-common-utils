#include "string_util.h"
#include <stdio.h>

int main(void) {

    // init string and print its struct fields
    String s1 = str("Hello");
    printf("text = \"%s\", length = %d, memory allocated = %d bytes\n", s1.text, s1.len, s1.cap);
    // text = "Hello", length = 5, memory allocated = 11 bytes

    // append then free suffix after
    String s2 = str(", ");
    str_append(&s1, &s2);
    str_free(&s2);

    // append and free suffix in one line
    String s3 = str("world!");
    str_append(&s1, &s3, .free_suffix=true);
    printf("%s\n", s1.text); // "Hello, world!"

    // prepend and free prefix in one line
    String s4 = str(" How are you?");
    str_prepend(&s1, &s4, .free_prefix=true);
    printf("%s\n", s4.text); // "Hello, world! How are you?"

    // concat multiple strings into one, with options to set which string to store the output in (defaults to first list item), and whether to free the others or not (defaults to false)
    String a = str("AAA");
    String b = str("BBB");
    String c = str("CCC");
    String d = str("DDD");
    String *parts[] = {&a, &b, &c, &d};
    str_concat(parts, .output_index=2, .free_others=true);
    printf("%s\n", c.text); // "AAABBBCCCDDD"
    String e = str("EEE");
    String f = str("FFF");
    String g = str("GGG");
    str_concat(((String *[]){&c, &e, &f, &g}));
    printf("%s\n", c.text); // "AAABBBCCCDDDEEEFFFGGG"
    str_free(&c);

    // free multiple strings at once
    str_free_all(((String *[]){&e, &f, &g}));

    return 0;
}
