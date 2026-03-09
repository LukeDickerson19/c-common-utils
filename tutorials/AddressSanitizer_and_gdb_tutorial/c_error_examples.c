#include <stdlib.h>


void heap_buffer_overflow_example() {
    int *x = malloc(4); // heap allocation
    x[1] = 10; // buffer overflow
    free(x);
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
    heap_buffer_overflow_example();
    // stack_buffer_overflow();
    // use_after_free();
    // global_buffer_overflow();
}
