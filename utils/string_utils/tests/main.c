#include <stdio.h>
#include "string_utils.h";

int main(void) {
    String s1 = str("hello", 20), s2 = str(" world", 12);
    append(s1, s2);
    printf("\"%s\"", s1.text);
    String s3 = str("foo", 5), s4 = str("bar", 10);
    prepend(s3, s4);
    printf("\"%s\"", s4);
    free_strings({s1, s2, s3, s4});
    return 0;
}
