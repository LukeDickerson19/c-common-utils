# string-util

> [!NOTE]
> This is an older version of the string_util code before it was made UTF-8 compatible. It only works with ASCII text.

#### DESCRIPTION

> Dynamic string utility written in C.

##### Main string struct and constructor function:
```c
typedef struct {
    char   *text;
    size_t  len;
    size_t  cap;
} String;

String str(const char *fmt, ...);
```

##### Features:
> - dynamic strings that double or halve the heap memory allocation (with malloc/free) as the string grows or shrinks
> - Functions:
>   - **Memory**:
>     - str_free
>     - str_clone
>   - **Mutation**:
>     - str_append
>     - str_prepend
>     - str_concat
>     - str_to_upper
>     - str_to_lower
>     - str_insert
>     - str_replace
>     - str_repeat
>     - str_remove
>     - str_trim
>     - str_trim_left
>     - str_trim_right
>   - **Query**:
>     - str_equals
>     - str_is_empty
>     - str_starts_with
>     - str_ends_with
>     - str_contains
>     - str_count
>     - str_index_of
>     - str_indices_of
>   - **Extract**:
>     - str_split
>     - str_slice


#### BUILD
```
cd c-common-utils/utils/string_utils/original_ascii_string_util/
cmake -S . -B build
cmake --build build
```


#### USAGE
Below is a quick example usage. The [tests/string_util_full_example.c](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/string_util/tests/string_util_full_example.c) file shows how to use all this string_util's features. See [include/string_util.h](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/string_util/include/string_util.h) for all function definitions and descriptions.
```c
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
```

#### EXAMPLE OUTPUT
```
[luke@luke utils]$ 
[luke@luke utils]$ 
[luke@luke utils]$ ./build/string_util/string_util_readme_example 
text = "Hello", length = 5, memory allocated = 11 bytes
text = "Hello, world", length = 12, memory allocated = 22 bytes
text = "... Hello, world", length = 16, memory allocated = 22 bytes
AAABBBCCCDDD
AAABBBCCCDDDEEEFFFGGG
[luke@luke utils]$ 
[luke@luke utils]$ 
```

