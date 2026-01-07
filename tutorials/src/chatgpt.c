#define _POSIX_C_SOURCE 200809L // tells the compiler to expose POSIX.1-2008 features, including sigaction and related definitions
#include <stdlib.h>
#include <stdio.h> // used for INT_MAX, INT_MIN, etc.
#include <errno.h> // used for errno variable
#include <string.h> // used for strerror
#include <limits.h>
#include <float.h>
#include <stdint.h> // used for int32_t, uint64_t
#include <inttypes.h> // used for PRIu64 (PRIdPTR ensures portable printing)
#include <signal.h> // used for signal(), sigaction() functions
#include <unistd.h>
#include "../include/chatgpt.h" // example header file
#include "../include/helper.h" // example dependency for make

/*
===============================================================================
   TYPICAL MEMORY SIZES BY PLATFORM (in bytes)
===============================================================================

Type                Linux 32   Linux 64   Windows 32  Windows 64  macOS 64  Android ARM32  Android ARM64  iOS 64
-------------------------------------------------------------------------------------------------------------
char                1          1          1           1           1         1              1               1
short               2          2          2           2           2         2              2               2
int                 4          4          4           4           4         4              4               4
long                4          8          4           4           8         4              8               8   (LP64 model)
long long           8          8          8           8           8         8              8               8
pointer (*)         4          8          4           8           8         4              8               8

Notes:
 - Unsigned versions weren’t included because typical memory size charts focus on byte width, and unsigned types use the same size. Unsigned max values are double the signed max values plus one.
 - Windows always uses ILP32/LLP64 → long = 4 bytes even on 64-bit.
 - Linux/macOS/iOS/Android 64-bit use LP64 → long = 8 bytes.
 - iOS is exclusively 64-bit today → no 32-bit support on modern devices.
===============================================================================
*/


// Declaration
void variable_types();
// Definition
void variable_types() {

    printf("\n==================== INTEGER LIMITS • MEMORY • FORMATS ====================\n\n");

    // int
    int i1 = 1;
    printf("int, example: %d\n", i1);
    printf("  Bytes              : %zu\n", sizeof(int));
    printf("  MIN                : %d\n", INT_MIN);
    printf("  MAX                : %d\n", INT_MAX);
    printf("  MIN (hex)          : %x\n", INT_MIN);
    printf("  MAX (hex)          : %x\n", INT_MAX);
    printf("  MIN (octal)        : %o\n", INT_MIN);
    printf("  MAX (octal)        : %o\n\n", INT_MAX);

    // unsigned int
    unsigned int i2 = 7;
    printf("unsigned int, example: %d\n", i2);
    printf("  Bytes              : %zu\n", sizeof(unsigned int));
    printf("  MIN                : %u\n", 0u);
    printf("  MAX                : %u\n", UINT_MAX);
    printf("  MAX (hex)          : %x\n", UINT_MAX);
    printf("  MAX (octal)        : %o\n", UINT_MAX);
    printf("\n");

    // short
    printf("short\n");
    printf("  Bytes              : %zu\n", sizeof(short));
    printf("  MIN                : %hd\n", SHRT_MIN);
    printf("  MAX                : %hd\n", SHRT_MAX);
    printf("  MIN (hex)          : %hx\n", SHRT_MIN);
    printf("  MAX (hex)          : %hx\n", SHRT_MAX);
    printf("  MIN (octal)        : %ho\n", SHRT_MIN);
    printf("  MAX (octal)        : %ho\n\n", SHRT_MAX);

    // unsigned short
    printf("unsigned short\n");
    printf("  Bytes              : %zu\n", sizeof(unsigned short));
    printf("  MIN                : %hu\n", 0);
    printf("  MAX                : %hu\n", USHRT_MAX);
    printf("  MIN (hex)          : %hx\n", 0);
    printf("  MAX (hex)          : %hx\n", USHRT_MAX);
    printf("  MIN (octal)        : %ho\n", 0);
    printf("  MAX (octal)        : %ho\n\n", USHRT_MAX);

    // long
    printf("long\n");
    printf("  Bytes              : %zu\n", sizeof(long));
    printf("  MIN                : %ld\n", LONG_MIN);
    printf("  MAX                : %ld\n", LONG_MAX);
    printf("  MIN (hex)          : %lx\n", LONG_MIN);
    printf("  MAX (hex)          : %lx\n", LONG_MAX);
    printf("  MIN (octal)        : %lo\n", LONG_MIN);
    printf("  MAX (octal)        : %lo\n\n", LONG_MAX);

    // unsigned long
    printf("unsigned long\n");
    printf("  Bytes              : %zu\n", sizeof(unsigned long));
    printf("  MIN                : %lu\n", 0ul);
    printf("  MAX                : %lu\n", ULONG_MAX);
    printf("  MAX (hex)          : %lx\n", ULONG_MAX);
    printf("  MAX (octal)        : %lo\n\n", ULONG_MAX);

    // long long
    printf("long long\n");
    printf("  Bytes              : %zu\n", sizeof(long long));
    printf("  MIN                : %lld\n", LLONG_MIN);
    printf("  MAX                : %lld\n", LLONG_MAX);
    printf("  MIN (hex)          : %llx\n", LLONG_MIN);
    printf("  MAX (hex)          : %llx\n", LLONG_MAX);
    printf("  MIN (octal)        : %llo\n", LLONG_MIN);
    printf("  MAX (octal)        : %llo\n\n", LLONG_MAX);

    // unsigned long long
    printf("unsigned long long\n");
    printf("  Bytes              : %zu\n", sizeof(unsigned long long));
    printf("  MIN                : %llu\n", 0ull);
    printf("  MAX                : %llu\n", ULLONG_MAX);
    printf("  MAX (hex)          : %llx\n", ULLONG_MAX);
    printf("  MAX (octal)        : %llo\n\n", ULLONG_MAX);

    // char
    char c = 'A';
    printf("char\n");
    printf("  Example value       : '%c' (%d) (hex: %x, octal: %o)\n", c, c, c, c);
    printf("  Bytes               : %zu\n", sizeof(char));
    printf("  MIN                 : %d\n", CHAR_MIN);
    printf("  MAX                 : %d\n", CHAR_MAX);
    printf("  MIN (hex)           : %x\n", CHAR_MIN);
    printf("  MAX (hex)           : %x\n", CHAR_MAX);
    printf("  MIN (octal)         : %o\n", CHAR_MIN);
    printf("  MAX (octal)         : %o\n\n", CHAR_MAX);

    // unsigned char
    unsigned char uc = 200;
    printf("unsigned char\n");
    printf("  Example value       : %u (hex: %x, octal: %o)\n", uc, uc, uc);
    printf("  Bytes               : %zu\n", sizeof(unsigned char));
    printf("  MIN                 : %u\n", 0u);
    printf("  MAX                 : %u\n", UCHAR_MAX);
    printf("  MAX (hex)           : %x\n", UCHAR_MAX);
    printf("  MAX (octal)         : %o\n\n", UCHAR_MAX);

    // float
    float f = 3.14f;
    printf("float\n");
    printf("  Example value       : %.6f\n", f);
    printf("  Bytes               : %zu\n", sizeof(float));
    printf("  MIN                 : %e\n", FLT_MIN);
    printf("  MAX                 : %e\n", FLT_MAX);
    printf("  Precision digits    : %d\n\n", FLT_DIG);

    // double
    double d = 3.141592653589793;
    printf("double\n");
    printf("  Example value       : %.15f\n", d);
    printf("  Bytes               : %zu\n", sizeof(double));
    printf("  MIN                 : %e\n", DBL_MIN);
    printf("  MAX                 : %e\n", DBL_MAX);
    printf("  Precision digits    : %d\n\n", DBL_DIG);

    // long double
    long double ld = 3.141592653589793238L;
    printf("long double\n");
    printf("  Example value       : %.18Lf\n", ld);
    printf("  Bytes               : %zu\n", sizeof(long double));
    printf("  MIN                 : %Le\n", LDBL_MIN);
    printf("  MAX                 : %Le\n", LDBL_MAX);
    printf("  Precision digits    : %d\n\n", LDBL_DIG);

    // int32_t: A signed 32-bit integer type, guaranteed to be exactly 32 bits wide. Defined in <stdint.h>
    int32_t fixed_width = -42;
    printf("int32t: %" PRId32 "\n", fixed_width);

    // uint64_t: An unsigned 64-bit integer type, guaranteed to be exactly 64 bits wide. Defined in <stdint.h>.
    // useful in cryptography
    uint64_t big_number = 18446744073709551615ULL;
    printf("uint64_t: %" PRIu64 "\n", big_number);

    // intptr_t: A signed integer type capable of holding a pointer. Defined in <stdint.h>.
    int z = 42;
    intptr_t ptr_val = (intptr_t)&z;
    int *ptr = (int *)ptr_val;
    printf("intptr_t: Address as integer: %" PRIdPTR "\n", ptr_val);

    // size_t: An unsigned integer type for sizes and counts. Defined in <stddef.h>.
    size_t length = 10;
    char *buffer = malloc(length);
    printf("size_t: allocated %zu bytes\n", length);
    free(buffer);

    char fixed_length_str[1024] = "";

    printf("==========================================================================\n");

	/* NOTE:
		In C, local function variables (including the main() function) are automatically “deleted” when the function returns (more precisely, their storage is reclaimed by moving the stack pointer). They are typically stored on the stack, so their lifetime is limited to the function call. When the function returns the stack pointer moves back, effectively “removing” these variables. The memory can be reused by other functions or future calls to the same function.

		C stores variables in different memory segments based on their scope and lifetime:
		    Data Segment:
                Description:
    		        The data segment is allocated once at program start, and freed when the program exits. The data is stored in fixed memory addresses which don't change for the lifetime of the program.
                It stores:
                    global variables
                    static variables
                    constants
				Lifetime:
                    entire program run
			Stack (aka Automatic Memory):
                Description:
                    The stack stores local variables defined within functions, and manages their memory automatically via function calls and returns
                It stores:
                    local variables
				Lifetime:
                    until the function exits
			Heap:
                Description:
    				The heap is used for dynamic memory allocation at runtime, and is managed manually via malloc(), calloc(), realloc(), free().
                It stores:
                    variables the program manually allocates
				Lifetime:
                    until you explicitly free it or the program ends

			A static variable in C is a local or global variable prefixed with "static" (ex: "static int x = 7;") whose lifetime is the entire program, but its scope may be limited.
                Global Static Variables are:
                    Declared outside functions with static
                    Limits visibility to the current file (internal linkage, as opposed to external linkage for normal global variables)
                Local static variables are:
                    Declared inside a function with static
                    Retains its value between function calls
                    Stored in data segment, not stack

            A static function is private to the file its written in.

            An "extern" variable in C is a 
	*/

}

// this functions declaration is in chatgpt.h
void unions() {
    printf("\n==================== UNIONS ====================\n\n");

    /* NOTES:
        A union overlays different types on the same memory address.
        You can interpret those bytes in any of the declared formats,
        but only the last-assigned field contains valid data.
    */

    // local union (see header file for global union example)
    union Data {
        int i;       // 4 bytes
        float f;     // 4 bytes
        char c;      // 1 byte
    };

    union Data x;

    x.i = 42;
    printf("i = %d\n", x.i);   // good
    printf("f = %f\n", x.f);   // garbage, same bytes interpreted as float

    x.f = 3.14f;
    printf("f = %f\n", x.f);   // good
    printf("i = %d\n", x.i);   // garbage again

    /* Output

        i = 42
        f = 0.000000  // undefined interpretation of the int bytes
        f = 3.140000
        i = 1078523331  // meaningless reinterpretation of the float bits

    */

    // tagged union usage (see header file for definition)
    TaggedUnion v;
    v.type = TYPE_INT;
    v.value.i = 10;
    if (v.type == TYPE_INT) {
        printf("tagged union %d\n", v.value.i);
    }

}

// these functions don't have a declaration because they're defined (aka implemented) before they're used
// NOTE: C does not allow function overloading (multiple fns w/ the same name, but different arguments and/or return types)
// NOTE: C functions are pass by value, not reference, so the args a function are working with are copies of the parameters passed to the function. So pass pointers when you need to need to change the actual data. 
void set_ptr(int **p) {
    static int x = 5;
    *p = &x;
}
void alloc_array(int **arr, int n) {
    *arr = malloc(n * sizeof(int));

    // Put example data into the array
    for (int i = 0; i < n; i++) {
        (*arr)[i] = i * i;  // example data
    }
}
typedef struct Node {
    int value;
    struct Node *next;
} Node;
void push(Node **head, int value) {
    // Why Node **head matters:
    // This function must change what head points to
    // A single Node *head would only modify a copy
    Node *n = malloc(sizeof(Node));
    if (n == NULL) {
        return;
    }
    n->value = value;
    n->next = *head;  // point to old head
    *head = n;        // update caller's head
}
void double_pointers() {
    // A double pointer is simply a pointer that points to another pointer.
    // Think of it as two levels of indirection.
    int x = 10;
    int *p = &x;     // pointer to int
    int **pp = &p;   // pointer to pointer to int

    printf("%p\n", pp); // address at pp
    printf("%p\n", *pp); // value of pp (which is &x)
    printf("%d\n", **pp);  // value of x

    // How double pointers are commonly used:
    
    // 1. Modify a pointer inside a function:
    //          C passes arguments by value, including pointers. If you want a function to change where a pointer points, you need a pointer to that pointer.
    int *p1 = NULL;
    set_ptr(&p1);
    printf("%d\n", *p1);  // 5

    // 2. Dynamic allocation of arrays or buffers
    // A double pointer is valuable because here it lets a function change the caller’s pointer.
    int *arr = NULL;
    int n = 10;
    alloc_array(&arr, n);
    if (arr == NULL) {
        printf("malloc failed\n");
        return;
    }
    printf("array contents:\n");
    for (int i = 0; i < n; i++) {
        printf("    arr[%d] = %d\n", i, arr[i]);
    }
    // Properly free the allocated memory
    free(arr);
    arr = NULL;  // good practice: avoid dangling pointer

    // 3. Arrays of strings (char **)
    char *words[] = {"hello", "world"};
    char **p3 = words;
    /*
        p3 points to the first element of the array
        *p3 is a char *
        **p3 is a char
    */
    printf("%p\n", p3); // p3 address
    printf("%s\n", *p3);  // 'hello'
    printf("%c\n", **p3);  // 'h'

    // 4. Linked lists and trees
    // Double pointers make insertion/deletion easier:
    Node *head = NULL;
    // Build example list: 3 -> 2 -> 1
    push(&head, 1);
    push(&head, 2);
    push(&head, 3);
    printf("linked list contents:\n");
    Node *cur = head;
    while (cur != NULL) {
        printf("    %d\n", cur->value);
        cur = cur->next;
    }
    // Free the list
    cur = head;
    while (cur != NULL) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    head = NULL;

}

// (C-Style) Object Oriented Programming
struct List {
    int value;
    struct List *next;
};
List *list_create(void) {
    return NULL; // Empty list
}
void list_push(List **list, int value) {
    // Allocate memory for new node
    List *new_node = (List *)malloc(sizeof(List));
    new_node->value = value;
    new_node->next = *list;
    *list = new_node; // Update head
}
void list_print(List *list) {
    while (list != NULL) {
        printf("%d -> ", list->value);
        list = list->next;
    }
    printf("NULL\n");
}
int list_contains(List *list, int value) {
    while (list != NULL) {
        if (list->value == value) {
            return 1; // Found
        }
        list = list->next;
    }
    return 0; // Not found
}
void list_free(List *list) {
    while (list != NULL) {
        List *temp = list;
        list = list->next;
        free(temp);
    }
}
void oop() {
    List *my_list = list_create(); // Create empty list

    // Push values
    list_push(&my_list, 10);
    list_push(&my_list, 20);
    list_push(&my_list, 30);

    // Print list: 30 -> 20 -> 10 -> NULL
    list_print(my_list);

    printf("Contains 20? %s\n", list_contains(my_list, 20) ? "Yes" : "No");
    printf("Contains 40? %s\n", list_contains(my_list, 40) ? "Yes" : "No");

    // delete list cause one of its functions uses malloc 
    list_free(my_list); // Free memory
}

// other stuff
volatile sig_atomic_t keep_running = 1; // Global flag to control the loop
void handle_sigint(int sig) {
    write(STDOUT_FILENO, "Caught SIGINT!\n", 25);
    keep_running = 0; // Set flag to exit the loop
    /* Async-signal-safe functions are functions that can be safely called from within a signal handler. Most functions are not async-signal-safe because they may use shared resources (e.g., malloc, printf), which can lead to race conditions or undefined behavior.
    
    List of Async-Signal-Safe Functions:
        _exit()
        write()
        read()
        sigaction()
        signal()
        sigprocmask()

    */
}
void other_advanced_stuff() {

    /* Undefined / Implementation-Defined / Unspecified Behavior

    Undefined behavior (UB):
        Undefined Behavior means the C standard does not specify what will happen. The program may crash, produce incorrect results, or behave unpredictably. Compilers are free to optimize code assuming UB does not occur, which can lead to surprising results.
        Examples:
            Signed integer overflow:
                Overflowing a signed integer is UB. The result may wrap around, or the program may crash.
                Example: */
                int a = INT_MAX; // 2147483647 on most systems
                a += 1; // Undefined Behavior: signed overflow
                printf("%d\n", a); // Could print -2147483648, or anything else
                /*
            Reading uninitialized variables:
                Reading an uninitialized variable is UB. The value could be garbage or zero, depending on the compiler.
                Example: */
                int x;
                printf("%d\n", x); // Undefined Behavior: x is uninitialized
                /*
            Using freed memory:
                Accessing memory after free() is UB. The program may crash or corrupt data.
                Example: */
                int *p = malloc(sizeof(int));
                *p = 42;
                free(p);
                printf("%d\n", *p); // Undefined Behavior: p is freed
                /*
            Out-of-bounds array access:
                Accessing an array beyond its bounds is UB. It may corrupt memory or crash.
                Example: */
                int arr[3] = {1, 2, 3};
                printf("%d\n", arr[5]); // Undefined Behavior: out of bounds
            /*
            Modifying and reading same variable without sequence point:
                Modifying a variable more than once between sequence points is UB.
                Example: */
                int i = 0;
                i = i++; // Undefined Behavior: i is modified twice
                /*

    Implementation-defined behavior
        char signedness:
            Whether char is signed or unsigned depends on the compiler.
            Example:
            */
            char c = 0xFF;
            if (c == -1) {
                puts("char is signed");
            } else {
                puts("char is unsigned");
            }
            /*
        Size of long:
            The size of long (e.g., 4 or 8 bytes) is implementation-defined.
            Example: */
            printf("Size of long: %zu bytes\n", sizeof(long));
            /*
        Bitfield layout:
            The order and padding of bitfields are implementation-defined.
            Example: */
            struct Bitfield {
                unsigned int a : 1;
                unsigned int b : 1;
            };
            printf("Size of Bitfield: %zu bytes\n", sizeof(struct Bitfield));
            /*

    Unspecified behavior
        Argument evaluation order:
            The order in which function arguments are evaluated is unspecified.
                Example: */
                int n = 0;
                int m = printf("%d %d\n", n++, n++); // Unspecified: could print 0 1 or 1 0
        /*

    Key Takeaways:
        Undefined Behavior:
            Anything can happen (crash, corruption, etc.).
        Implementation-Defined Behavior:
            The compiler decides, but it’s consistent.
        Unspecified Behavior:
            The compiler chooses from valid options, but the result may vary.
    
    Best Practice:
        Avoid UB and unspecified behavior for portable, reliable code. Use compiler flags (e.g., -Wall -Wextra) to catch potential issues.
    
    */


    /*
    
    Aliasing rules (why char* is special)
        C enforces strict aliasing rules: pointers of different types cannot alias (point to) the same memory. Violating this rule can lead to unexpected optimizations or crashes. However C allows pointers of type char *, unsigned char *, or void * to alias any other type. This is because char is considered the "universal donor" for type-punning and memory inspection.

        Example: */
            int d = 0x12345678;
            char *cp = (char *)&d; // Allowed: char* can alias any type
            // You can inspect the bytes of 'a' using cp:
            for (int i = 0; i < sizeof(int); i++) {
                printf("%02x ", cp[i]); // Prints: 78 56 34 12 (on little-endian systems)
            }

    /*
    Type-Punning:
        Type-Punning is accessing an object through a pointer of a different type. Violating effective type rules can lead to undefined behavior.

        Its only safe when:
            Type-punning via char* because char* is exempt from strict aliasing.
            Union-based type-punning: Defined behavior in C (but not C++).
                Example (Union): */
                    union IntFloat {
                        int i;
                        float f;
                    };
                    union IntFloat u;
                    u.i = 0x12345678;
                    printf("%f\n", u.f); // Defined behavior in C
    /*
    Direct Pointer Casting (Undefined Behavior):
        Casting a pointer to an unrelated type and dereferencing it violates strict aliasing rules and is undefined behavior.
        Example (Undefined Behavior): */
            int b = 10;
            double *dp = (double *)&b; // Undefined behavior if dereferenced
            *dp = 3.14; // NASAL DEMONS MAY APPEAR

    // Const Correctness
    const int *p1;      // pointer to const int
    int * const p2;     // const pointer to int
    const int * const p3;

    /* Error Handling Patterns

    Return Codes:
        Functions return a specific value (often an integer) to indicate success or failure. Best for functions where all possible return values are valid, and an extra parameter is needed for error status.

    Sentinel Values:
        A special value (e.g., NULL, -1, or a reserved constant) is returned to indicate an error. Useful for simple functions where the return type can accommodate a sentinel.

    Usage Rules of "errno"
        What is errno?
            A global variable (int errno) defined in <errno.h> for reporting errors by library functions (e.g.,  fopen, malloc).
        Rules:
            Only check errno if the function's return value indicates an error.
            Never set errno to 0 in your code; only library functions do this.
            errno is thread-local in modern systems.

        Examples: */

            // file does not exist
            FILE *file = fopen("nonexistent.txt", "r");
            if (file == NULL) {
                perror("Error opening file"); // Prints: "Error opening file: No such file or directory"
                if (errno == ENOENT) {
                    printf("File does not exist.\n");
                }
            }

        /*
            // out of memory
            int *arr = malloc(10000000000000000000); // Intentionally too large
            if (arr == NULL) {
                printf("Memory allocation failed: %s\n", strerror(errno));
                // Example output: "Memory allocation failed: Cannot allocate memory"
            }

        Best Practices:
            Use perror() or strerror(errno) to print error messages.
            Reset errno to 0 before calling a function if you need to distinguish between "no error" and "error not set."

    3. Error Propagation
        Definition:
            Passing error information up the call stack so higher-level code can handle it.
        Key Points:
            Propagate errors by returning error codes or sentinels.
            Clean up resources (e.g., close files) before returning.

    4. When to Use "assert"
        Definition:
            assert (from <assert.h>) is a macro for debugging. It aborts the program if a condition is false.
        Use Cases:
            Debugging: Catch logical errors during development.
            Invariants: Verify assumptions that must always hold (e.g., "this pointer is not NULL").
        Rules:
            Never use assert for runtime error handling (it's disabled with NDEBUG).
            Use for internal consistency checks, not user input validation.

        Example: */
            assert(1 < 2);
    /*

    Signals:
        Signals are software interrupts sent to a program to indicate that an important event has occurred. They can be generated by the system (e.g., segmentation fault) or sent between processes (e.g., kill command). Handling signals allows programs to respond to asynchronous events gracefully.

        The signal() function is a simple way to set a signal handler for a specific signal. It is defined in <signal.h>.
            Example: */
                signal(SIGINT, handle_sigint);
                write(STDOUT_FILENO, "Waiting for SIGINT (signal())...\n", 34);
                while (keep_running) {
                    sleep(1);
                }

            /*      When you press Ctrl+C, the program prints:
                    "Caught signal 2 (SIGINT)"

        The sigaction() function provides more control over signal handling, including the ability to block additional signals during handler execution. It is the preferred way to handle signals in modern C.
            Example: */
                struct sigaction sa;
                sa.sa_handler = handle_sigint; // Use the named function
                sigemptyset(&sa.sa_mask);
                sa.sa_flags = 0;
                sigaction(SIGINT, &sa, NULL);
                write(STDOUT_FILENO, "Waiting for SIGINT (sigaction())...\n", 37);
                keep_running = 1; // Reset the flag
                while (keep_running) {
                    sleep(1);
                }
        /*

    Common Security Pitfalls:

        Buffer overflows:
            A buffer overflow occurs when a program writes more data to a buffer (e.g., an array) than it can hold, overwriting adjacent memory. This can corrupt data, crash the program, or allow arbitrary/malicious code execution/injection.

            Example:

                char large_input[20] = "ThisIsWayTooLong";
                char buffer[10];
                strcpy(buffer, input); // No bounds checking

            Mitigation Strategy:
                Use bounds-checked functions like strncpy or snprintf.

        Use-after-free:
            Accessing memory after it has been freed can lead to undefined behavior, crashes, or security vulnerabilities. The freed memory might be reused for other purposes, and accessing it can corrupt data or expose sensitive information.

            Example:
                int *ptr = malloc(sizeof(int));
                *ptr = 42;
                free(ptr);
                // ptr is now dangling
                printf("%d\n", *ptr); // Use-after-free!
                return 0;

            Mitigation Strategy:
                Use-After-Free/Double Free: Set pointers to NULL after freeing and avoid reusing them.

        Double free:
            Freeing the same block of memory twice can corrupt the heap, leading to crashes or arbitrary code execution.
            
            Example:
                int *ptr = malloc(sizeof(int));
                free(ptr);
                free(ptr); // Double free!

            Mitigation Strategy:
                Compile with flags like -Wall -Wextra (GCC/Clang) to catch potential issues.
                Tools like Clang’s AddressSanitizer (ASan), Valgrind, or Coverity can detect double free bugs during development and testing.
                Set pointers to NULL after freeing
                Create a wrapper function that checks if a pointer is already NULL before freeing it
                Some allocators (e.g., glibc’s malloc) include debug modes to detect double frees. Enable them during testing.

        Integer overflow:
            An integer overflow occurs when an arithmetic operation exceeds the maximum (or minimum) value a variable can hold, wrapping around to an unexpected value. Overflows can bypass security checks (e.g., size validations) or cause buffer overflows.

            Example:
                unsigned int a = UINT_MAX; // Maximum value for unsigned int
                unsigned int b = 1;
                unsigned int result = a + b; // Overflow: wraps to 0
                printf("Result: %u\n", result); // Prints 0

            Mitigation Strategy:
                Buffer Overflows: Use bounds-checked functions like snprintf.
                Use-After-Free/Double Free: Set pointers to NULL after freeing and avoid reusing them.
                Integer Overflows: Use larger data types or check for overflow before operations.
                Format Strings: Never pass user input as the format string. Use printf("%s", user_input) instead.

        Format string vulnerabilities
            Format string vulnerabilities occur when user-controlled input is passed as the format string to functions like printf, allowing attackers to read or write arbitrary memory.
            
            Example:
                #include <stdio.h>

                int main(int argc, char *argv[]) {
                    if (argc > 1) {
                        printf(argv[1]); // User controls format string
                    }
                    return 0;
                }

            Mitigation Strategy:
                Format Strings: Never pass user input as the format string. Use printf("%s", user_input) instead.

    */

}


int main() {

    // print C version
    #ifdef __STDC_VERSION__
        printf("C Standard: %ld\n", __STDC_VERSION__);
    #else
        printf("Pre-C89 (no __STDC_VERSION__ defined)\n");
    #endif

	// variable_types();
    // unions();
    // double_pointers();
    // oop();
    // greet("User");  // Call helper function from helper.c
    other_advanced_stuff();
	return 0;
}
