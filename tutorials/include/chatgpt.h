

/*

What is a header file?
    A header file (.h extension) is a file that contains declarations, not implementations.

Typical contents:
    - Function declarations (prototypes)
    - struct, enum, union definitions
    - typedefs
    - #define macros
    - extern global variables

Why headers exist
    C compilation happens in separate steps:
        Preprocessing (#include, #define)
        Compilation (.c → .o)
        Linking (.o files → executable)

    Each .c file is compiled independently.
    Headers let multiple .c files agree on:
        - Function signatures
        - Struct layouts
        - Shared constants

*/

// Example function declaration
#ifndef UNION_UTILS_H
#define UNION_UTILS_H

void unions();

#endif

/*

The function unions() can be used in any .c file that contains the line:
    #include "chatgpt.h"
"#include" does literal text substitution. The preprocessor copy/pastes the contents of the header into the .c file before compilation. This is why multiple include lines can cause "redefinition" errors without header guards. Header guards cause the file contents within them to only get text substituted into .c file(s) if they haven't already been substituted yet.
    Example:
        #ifndef X_H
        #define X_H

        int x;

        #endif

        "ifndef" stands for "if not defined"
    
    Alternative Example (non-standard but common):
        #pragma once

There can be multiple header guards per headerfile but its simpler to just make 1 cause the whole header file gets imported.

*/

#ifndef CHATGPT_H
#define CHATGPT_H

// example struct
typedef struct {
    int x;
    int y;
} Point;                                                                            

// example enum
enum Color {
    RED,
    GREEN,
    BLUE
};

// example union
union Number {
    int i;
    float f;
};

// Example tagged Union - taked unions are unions with a "tag" showing the current data type of the union
typedef enum {
    TYPE_INT,
    TYPE_FLOAT
} ValueType;

typedef struct {
    ValueType type;
    union {
        int i;
        float f;
    } value;
} TaggedUnion;

// example macro
/* Macros:
    A macro is a text substitution performed by the preprocessor before the compiler sees your code.

    Object-like macro (constant replacement):
        #define PI 3.14159
        Usage:
            float area = PI * r * r;
        Preprocessor output:
            float area = 3.14159 * r * r;

    Function-like macro:
        #define SQUARE(x) x * x
        Usage:
            int y = SQUARE(a + b);
        What actually happens:
            int y = a + b * a + b;  // WRONG
        The classic fix: parentheses
            #define SQUARE(x) ((x) * (x))
        Now this happens:
            int y = ((a + b) * (a + b)) // CORRECT

        Another example pitfall:
            #define INC(x) ((x) + 1)
            Usage:
                int i = 5;
                INC(i++);   // expands to ((i++) + 1)
                Now i++ runs once — but with more complex macros it may run multiple times.

    Macros vs const
        Prefer const when possible:
        #define MAX_SIZE 100
        vs
        const int MAX_SIZE = 100;

    Why const is better:
        Type-safe
        Appears in debugger
        Scoped

    Use macros when:
        Value must be known at preprocessing time
        Used in #if
        Conditional compilation:
            #define DEBUG 1

            #if DEBUG
                printf("Debug mode\n");
            #endif

            Or:

            #ifdef DEBUG
                printf("Debug mode\n");
            #endif
        Debug logging
        Platform-specific code
            #ifdef _WIN32
                #define PATH_SEPARATOR '\\'
            #else
                #define PATH_SEPARATOR '/'
            #endif
        Feature flags

    You can remove a macro definition:
        #undef PI
        Useful to avoid name clashes.

    Macro operators (advanced but important)
        Stringification (#)
            #define STR(x) #x
            printf("%s\n", STR(hello)); // outputs: hello

        Token pasting (##)
            #define MAKE_VAR(name) int var_##name
            MAKE_VAR(foo); // Expands to: int var_foo;

        Header guards (macros in real life)
            #ifndef MY_HEADER_H
            #define MY_HEADER_H
            // declarations
            #endif // Prevents multiple inclusion.

    When macros are actually the right tool:
        Header guards
        Conditional compilation
        Compile-time constants
        Small inline-like expressions with care
        Platform/compiler detection

    Avoid macros for:
        Complex logic
        Anything that needs type safety
        Anything involving side effects

*/
#define MAX(a,b) ((a) > (b) ? (a) : (b));

// example external global variable
// "extern" means defined somewhere external, so "int global_count = 0" will go in another .c file, and "extern int global_count" can go it many different files. It can only be defined in one location, else you'll cause a linker error.
extern int global_count;

// C-style OOP (Headers and encapsulation)
typedef struct List List; // define object name
List *list_create(void); // initialize object and return pointer to object
void list_push(List **list, int value); // object function
void list_print(List *list);
int list_contains(List *list, int value);
void list_free(List *list);



#endif



