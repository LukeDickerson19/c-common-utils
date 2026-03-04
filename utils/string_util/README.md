# string-utils

#### DESCRIPTION

> Dynamic string utility written in C.

##### Main struct and string init function:
```c
typedef struct {
    char   *text;
    size_t  len;
    size_t  cap;
} String;

String str(const char *fmt, ...);
```

##### Features:
> - dynamic strings that double/halve heap memory allocation as the string grows/shrinks
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
>     - str_is_empty
>     - str_starts_with
>     - str_ends_with
>     - str_contains
>     - str_index_of
>     - str_indices_of
>     - str_equals
>     - str_count
>   - **Extract**:
>     - str_split
>     - str_slice
>
> - see [include/string_util.h](https://github.com/LukeDickerson19/c-common-utils/tree/master/utils/string_util/include/string_util.h) for function definitions and descriptions.


#### USAGE
Below is a quick example usage. The [tests/main.c](https://github.com/LukeDickerson19/c-common-utils/tree/master/utils/string_util/include/string_util.h) file shows how to use all this string_util's features.
```c
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
    String a = str("AAA"), b = str("BBB"), c = str("CCC"), d = str("DDD");
    String *parts[] = {&a, &b, &c, &d};
    str_concat(parts, .output_index=2, .free_others=true);
    printf("%s\n", c.text); // "AAABBBCCCDDD"
    String e = str("EEE");
    String f = str("FFF");
    String g = str("GGG");
    str_concat(((String *[]){&c, &e, &f, &g}));
    printf("%s\n", c.text); // "AAABBBCCCDDDEEEFFFGGG"

    // free multiple strings at once
    str_free(&c, &e, &f, &g);

    return 0;
}

```

#### EXAMPLE OUTPUT
```
[luke@luke string_util]$ 
[luke@luke string_util]$ 
[luke@luke string_util]$ ./build/readme_example 
text = "Hello", length = 5, memory allocated = 11 bytes
Hello, world!
Hello, world! How are you?
AAABBBCCCDDD
AAABBBCCCDDDEEEFFFGGG
[luke@luke string_util]$ 
[luke@luke string_util]$ 
```

#### BUILD
```
[luke@luke string_util]$ cmake -S . -B build
[luke@luke string_util]$ cmake --build build
```

