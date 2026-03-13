# string-util

#### DESCRIPTION

> UTF-8 compatible dynamic string utility written in C.

##### Main string struct:
```c
typedef struct String {
    char   *text;
    size_t  len;
    size_t  bytes;
    size_t  cap;
} String;
```

##### Features:
> - dynamic strings that double or halve the heap memory allocation (with malloc/free) as the string grows or shrinks
> - UTF-8 compatible using the [utf8proc](https://juliastrings.github.io/utf8proc/) library dependency "a small, clean C library that provides Unicode normalization, case-folding, and other operations [used for accurate string comparison and searching]". So all text args of the below functions assume UTF-8 input. 
> - Functions:
>   - **Memory**:
>     - str()
>     - str_free()
>     - str_clone()
>     - str_info()
>   - **Mutation**:
>     - str_append()
>     - str_prepend()
>     - str_concat()
>     - str_to_upper()
>     - str_to_lower()
>     - str_insert()
>     - str_replace()
>     - str_repeat()
>     - str_remove()
>     - str_trim()
>     - str_trim_left()
>     - str_trim_right()
>   - **Query**:
>     - str_equals()
>     - str_is_empty()
>     - str_starts_with()
>     - str_ends_with()
>     - str_contains()
>     - str_count()
>     - str_index_of()
>     - str_indices_of()
>   - **Extract**:
>     - str_split()
>     - str_slice()

#### BUILD & RUN

**Linux:**
```bash
cd c-common-utils/utils
cmake -S . -B build -DBUILD_STRING_UTIL=ON
cmake --build build
./build/string_util/string_util_readme_example
./build/string_util/string_util_full_example
```

**Windows** (run from "x64 Native Tools Command Prompt for VS"):
```bat
cd c-common-utils\utils
cmake -S . -B build -DBUILD_STRING_UTIL=ON
cmake --build build --config Release
cd build\string_util\Release
chcp 65001
.\string_util_readme_example.exe
.\string_util_full_example.exe
```

#### USAGE
Below is a quick example usage. The [tests/string_util_full_example.c](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/string_util/tests/string_util_full_example.c) file shows how to use all this string_util's features. See [include/string_util.h](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/string_util/include/string_util.h) for all function definitions and descriptions.
```c
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
```

#### EXAMPLE OUTPUT
```
[luke@luke utils]$ 
[luke@luke utils]$ 
[luke@luke utils]$ ./build/string_util/string_util_readme_example 
text="Hello", len=5, bytes=5, cap=11, String struct size=32, total size=43 bytes
text="Hello, world", len=12, bytes=12, cap=22, String struct size=32, total size=54 bytes
text="... Hello, world", len=16, bytes=16, cap=22, String struct size=32, total size=54 bytes
Hello東京¡café!naïve😀🍓🌍❌✅
Hello東京¡café!naïve😀🍓🌍❌✅→↙●■▲∞∑√★♥🔒🔓
[luke@luke utils]$ 
[luke@luke utils]$ 
```

#### DEPENDENCIES

utf8proc v2.11.3 is vendored in `string_util/external/utf8proc/` and included in the repo — no installation required.
