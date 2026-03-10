#include <stdlib.h>


void heap_buffer_overflow_example(int example_arg) {
    int *example_local_fn_var = malloc(8); // heap allocation
    example_local_fn_var[0] = 777; // buffer overflow
    example_local_fn_var[1] = 888; // buffer overflow

    example_local_fn_var[1] = example_arg; // buffer overflow
    free(example_local_fn_var);
}

void stack_buffer_overflow() {
    int arr[3]; // stack allocation
    arr[5] = 1; // buffer overflow
}

void use_after_free() {
    int *ptr;
    *ptr = 5;
    free(ptr);
    *ptr = 10; // use after free()
}

void global_buffer_overflow() {
    char s[5];
    s[10] = 'A';
}

int main() {
    heap_buffer_overflow_example(10);
    // stack_buffer_overflow();
    // use_after_free();
    // global_buffer_overflow();
}
