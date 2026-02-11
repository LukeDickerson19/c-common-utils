// test.c
#include "string_utils.h"
#include <stdio.h>
#include <assert.h>   // for assert (optional - can remove if you prefer)

void print_string(const String *s, const char *label) {
    if (!s || !s->text) {
        printf("    %s: (null or invalid)\n", label);
        return;
    }
    printf("    %s: \"%s\"  (len=%zu, cap=%zu)\n", label, s->text, s->len, s->cap);
}

void test_str_init(void) {
    printf("\n=== Basic creation and print ===\n");

    String a = str("Hello");
    print_string(&a, "a");

    String b = str(" world!");
    print_string(&b, "b");

    String empty = str("");
    print_string(&empty, "empty");

    free_string(&a);
    free_string(&b);
    free_string(&empty);
}

void test_append(void) {
    printf("\n=== Append tests ===\n");

    String base = str("Hello");

    // Append without free
    String suffix1 = str(", how are you?");
    append(&base, &suffix1);
    print_string(&base, "base string after append (no free)");
    print_string(&suffix1, "suffix1 (should still exist)");

    // Append with free
    String suffix2 = str(" I'm fine.");
    append(&base, &suffix2, .free_suffix = true);
    print_string(&base, "base string after append + free");
    print_string(&suffix2, "suffix2 (should be freed/reset)");

    free_string(&base);
}

void test_prepend(void) {
    printf("\n=== Prepend tests ===\n");

    String base = str("world!");

    // Prepend without free
    String prefix1 = str("Hello, ");
    prepend(&prefix1, &base);
    print_string(&base, "base string after prepend (no free)");
    print_string(&prefix1, "prefix1 (should still exist)");

    // Prepend with free
    String prefix2 = str("Goodbye cruel ");
    prepend(&prefix2, &base, .free_prefix = true);
    print_string(&base, "base string after prepend + free");
    print_string(&prefix2, "prefix2 (should be freed/reset)");

    free_string(&base);
}

void test_concat(void) {
    printf("\n=== Concat tests ===\n");

    String a = str("Hello");
    String b = str(", ");
    String c = str("beautiful");
    String d = str(" world!");

    String *parts1[] = {&a, &b, &c, &d, NULL};

    // Classic: concat into first, free others
    concat(parts1, .free_others = true);
    print_string(&a, "string a after concat into string at index 0 + free others");
    print_string(&b, "string b (should be freed)");
    print_string(&c, "string c (should be freed)");
    print_string(&d, "string d (should be freed)");

    // Reset for next test
    a = str("One");
    b = str("Two");
    c = str("Three");
    d = str("Four");

    String *parts2[] = {&a, &b, &c, &d, NULL};

    // Concat into index 2, do NOT free others
    concat(parts2, .output_index = 2);
    print_string(&c, "string c after concat into string at index 2 (no free)");
    print_string(&a, "string a (still exists)");
    print_string(&b, "string b (still exists)");
    print_string(&d, "string d (still exists)");

    // Clean up remaining
    free_strings(parts2);
}

void test_edge_cases(void) {
    printf("\n=== Edge cases ===\n");

    // Empty string append
    String empty = str("");
    String add = str("something");
    append(&empty, &add, .free_suffix = true);
    print_string(&empty, "empty + something");

    // Single string concat
    String single = str("alone");
    String *one[] = {&single, NULL};
    concat(one);
    print_string(&single, "single after concat (no change)");

    // Invalid cases (should return -1)
    int res = append(NULL, &add);
    printf("    append(NULL, ...) returned %d (expected -1)\n", res);

    free_string(&empty);
    free_string(&single);
}

int main(void) {
    printf("    === String Utils Test ===\n");

    test_str_init();
    test_append();
    test_prepend();
    test_concat();
    test_edge_cases();

    printf("    \nAll tests completed.\n");
    return 0;
}

