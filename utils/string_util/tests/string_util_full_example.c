// test.c
#include "string_util.h"
#include <stdio.h>  // for printf
#include <assert.h> // for assert (optional - can remove if you prefer)

void print_string(const String *s, const char *label) {
    if (!s || !s->text) {
        printf("    %s: (null or invalid)\n", label);
        return;
    }
    printf("    %s: \"%s\"  (len=%zu, cap=%zu)\n", label, s->text, s->len, s->cap);
}

void test_str_init(void) {
    printf("\n=== Basic creation and print ===\n");

    // Simple strings
    String a = str("Hello");
    print_string(&a, "a");

    String b = str(" world!");
    print_string(&b, "b");

    String empty = str("");
    print_string(&empty, "empty");

    // Formatted strings
    const char *name = "Alice";
    int age = 30;

    String f1 = str("Name: %s", name);
    print_string(&f1, "f1 (formatted)");

    String f2 = str("Name: %s, Age: %d", name, age);
    print_string(&f2, "f2 (formatted)");

    String f3 = str("Numbers: %d, %d, %d", 1, 2, 3);
    print_string(&f3, "f3 (formatted)");

    // Clean up
    str_free(&a, &b, &empty, &f1, &f2, &f3);
}

void test_str_clone(void) {
    printf("\n=== Clone tests ===\n");
    String s1 = str("hello world");
    String s2 = str_clone(&s1);

    print_string(&s1, "s1");
    print_string(&s2, "s2");

    printf("    insert substring into s1\n");
    String sub = str(", cruel");
    str_insert(&s1, &sub, 5);
    print_string(&s1, "s1");
    print_string(&s2, "s2");

    str_free(&s1, &s2, &sub);
}

void test_append(void) {
    printf("\n=== Append tests ===\n");

    String base = str("Hello");

    // Append without free
    String suffix1 = str(", how are you?");
    str_append(&base, &suffix1);
    print_string(&base, "base string after append (no free)");
    print_string(&suffix1, "suffix1 (should still exist)");

    // Append with free
    String suffix2 = str(" I'm fine.");
    str_append(&base, &suffix2, .free_suffix = true);
    print_string(&base, "base string after append + free");
    print_string(&suffix2, "suffix2 (should be freed/reset)");


    // Empty string append
    String empty = str("");
    String add = str("something");
    str_append(&empty, &add, .free_suffix = true);
    print_string(&empty, "empty + something");

    // Invalid cases (should return -1)
    int res = str_append(NULL, &add);
    printf("    append(NULL, ...) returned %d (expected -1)\n", res);

    str_free(&base, &empty);
}

void test_prepend(void) {
    printf("\n=== Prepend tests ===\n");

    String base = str("world!");

    // Prepend without free
    String prefix1 = str("Hello, ");
    str_prepend(&prefix1, &base);
    print_string(&base, "base string after prepend (no free)");
    print_string(&prefix1, "prefix1 (should still exist)");

    // Prepend with free
    String prefix2 = str("Goodbye cruel ");
    str_prepend(&prefix2, &base, .free_prefix = true);
    print_string(&base, "base string after prepend + free");
    print_string(&prefix2, "prefix2 (should be freed/reset)");

    str_free(&base);
}

void test_concat(void) {
    printf("\n=== Concat tests ===\n");

    // --- Test 1: classic concat into first + free others ---
    String a = str("Hello");
    String b = str(", ");
    String c = str("beautiful");
    String d = str(" world!");

    String *parts1[] = {&a, &b, &c, &d};

    print_string(&a, "string a");
    print_string(&b, "string b");
    print_string(&c, "string c");
    print_string(&d, "string d");

    str_concat(parts1, .free_others = true);
    printf("\n");
    print_string(&a, "string a after concat into string at index 0 + free others");
    print_string(&b, "string b (should be freed)");
    print_string(&c, "string c (should be freed)");
    print_string(&d, "string d (should be freed)");

    str_free(&a);

    // --- Test 2: concat into index 2, do NOT free others ---
    a = str("One");
    b = str("Two");
    c = str("Three");
    d = str("Four");

    String *parts2[] = {&a, &b, &c, &d};

    str_concat(parts2, .output_index = 2);
    print_string(&c, "string c after concat into string at index 2 (no free)");
    print_string(&a, "string a (still exists)");
    print_string(&b, "string b (still exists)");
    print_string(&d, "string d (still exists)");

    str_free(&a, &b, &c, &d);

    // --- Test 3: concat with separator ---
    a = str("apple");
    b = str("banana");
    c = str("cherry");
    d = str("date");
    String sep = str(" | ");

    String *parts3[] = {&a, &b, &c, &d};
    str_concat(parts3, .sep = &sep, .free_others = false);

    print_string(&a, "string a after concat with separator ' | '");
    print_string(&b, "string b (should still exist)");
    print_string(&c, "string c (should still exist)");
    print_string(&d, "string d (should still exist)");
    print_string(&sep, "separator string (unchanged)");

    str_free(&a, &b, &c, &d, &sep);

    // --- Test 4: single string concat (should remain unchanged) ---
    String single = str("alone");
    String *one[] = {&single};
    str_concat(one);
    print_string(&single, "single after concat (no change)");
    str_free(&single);
}

void test_is_empty(void) {
    printf("\n=== is_empty tests ===\n");

    String empty = str("");
    String nonempty = str("Hello");

    printf("    empty: %d (expected 1)\n", str_is_empty(&empty));
    printf("    nonempty: %d (expected 0)\n", str_is_empty(&nonempty));

    // Edge case: NULL pointer
    String *null_ptr = NULL;
    printf("    NULL pointer: %d (expected 1)\n", str_is_empty(null_ptr));

    str_free(&empty, &nonempty);
}

void test_starts_and_ends_with(void) {
    printf("\n=== starts_with / ends_with tests ===\n");

    String s = str("hello world");
    String pre1 = str("hello");
    String pre2 = str("world");
    String suf1 = str("world");
    String suf2 = str("hello");
    String long_prefix = str("hello world!!!"); // longer than s
    printf("    \"%s\" starts with \"%s\": %d (expected 1)\n", s.text, pre1.text, str_starts_with(&s, &pre1));
    printf("    \"%s\" starts with \"%s\": %d (expected 0)\n", s.text, pre2.text, str_starts_with(&s, &pre2));
    printf("    \"%s\" ends with \"%s\":   %d (expected 1)\n", s.text, suf1.text, str_ends_with(&s, &suf1));
    printf("    \"%s\" ends with \"%s\":   %d (expected 0)\n", s.text, suf2.text, str_ends_with(&s, &suf2));
    printf("    \"%s\" starts with \"%s\": %d (expected 0)\n", s.text, long_prefix.text, str_starts_with(&s, &long_prefix));

    // Edge cases: empty string
    String empty = str("");
    printf("    \"%s\" starts with \"%s\": %d (expected 1)\n", empty.text, empty.text, str_starts_with(&empty, &empty));
    printf("    \"%s\" ends with \"%s\":   %d (expected 1)\n", empty.text, empty.text, str_ends_with(&empty, &empty));

    str_free(&s, &pre1, &pre2, &suf1, &suf2, &long_prefix, &empty);
}

void test_to_upper_to_lower(void) {
    printf("\n=== to_upper / to_lower tests ===\n");

    String s1 = str("Hello World!");
    print_string(&s1, "s1");
    str_to_upper(&s1);
    print_string(&s1, "str_to_upper(&s1)");
    str_to_lower(&s1);
    print_string(&s1, "str_to_lower(&s1)");

    // Edge cases: empty string
    String empty = str("");
    print_string(&empty, "empty");
    str_to_upper(&empty);
    print_string(&empty, "str_to_upper(&empty)");
    str_to_lower(&empty);
    print_string(&empty, "str_to_lower(&empty)");

    // Mixed characters and numbers
    String mixed = str("123 AbC! xyz");
    print_string(&mixed, "mixed characters and numbers");
    str_to_upper(&mixed);
    print_string(&mixed, "str_to_upper(&mixed)");
    str_to_lower(&mixed);
    print_string(&mixed, "str_to_lower(&mixed)");

    // Clean up
    str_free(&s1, &empty, &mixed);
}

void test_contains(void) {
    printf("\n=== contains tests ===\n");

    String s = str("hello world");
    String sub1 = str("hello");
    String sub2 = str("world");
    String sub3 = str("o w");
    String sub4 = str("WORLD");
    String sub_empty = str("");

    printf("    \"%s\" contains \"%s\": %d (expected 1)\n", s.text, sub1.text, str_contains(&s, &sub1));
    printf("    \"%s\" contains \"%s\": %d (expected 1)\n", s.text, sub2.text, str_contains(&s, &sub2));
    printf("    \"%s\" contains \"%s\": %d (expected 1)\n", s.text, sub3.text, str_contains(&s, &sub3));
    printf("    \"%s\" contains \"%s\": %d (expected 0)\n", s.text, sub4.text, str_contains(&s, &sub4));
    printf("    \"%s\" contains \"%s\": %d (expected 1)\n", s.text, sub_empty.text, str_contains(&s, &sub_empty));

    // Edge cases: empty string
    String empty = str("");
    printf("    \"%s\" contains \"%s\": %d (expected 1)\n", empty.text, sub_empty.text, str_contains(&empty, &sub_empty));
    printf("    \"%s\" contains \"%s\": %d (expected 0)\n", empty.text, s.text, str_contains(&empty, &s));

    str_free(&s, &sub1, &sub2, &sub3, &sub4, &sub_empty, &empty);

    // ---- Longer pattern test ("ababcabcabababd") ----
    String s2 = str("ababcabcabababd");
    String sub2_1 = str("ababd");
    String sub2_2 = str("abcab");
    String sub2_3 = str("abcd");
    String sub2_empty = str("");

    printf("    \"%s\" contains \"%s\": %d (expected 1)\n", s2.text, sub2_1.text, str_contains(&s2, &sub2_1));
    printf("    \"%s\" contains \"%s\": %d (expected 1)\n", s2.text, sub2_2.text, str_contains(&s2, &sub2_2));
    printf("    \"%s\" contains \"%s\": %d (expected 0)\n", s2.text, sub2_3.text, str_contains(&s2, &sub2_3));
    printf("    \"%s\" contains \"%s\": %d (expected 1)\n", s2.text, sub2_empty.text, str_contains(&s2, &sub2_empty));

    str_free(&s2, &sub2_1, &sub2_2, &sub2_3, &sub2_empty);

}

void test_index_functions(void) {
    printf("\n=== Index tests ===\n");

    String s = str("abababacaca");
    String sub1 = str("ab");
    String sub2 = str("ac");
    String sub3 = str("xyz");

    // first and last
    printf("    First \"ab\" in \"%s\": %d (expected 0)\n", s.text, str_index_of(&s, &sub1, "first"));
    printf("    Last  \"ab\" in \"%s\": %d (expected 4)\n", s.text, str_index_of(&s, &sub1, "last"));
    printf("    First \"ac\" in \"%s\": %d (expected 6)\n", s.text, str_index_of(&s, &sub2, "first"));
    printf("    \"xyz\" not found:            %d (expected -1)\n", str_index_of(&s, &sub3, "first"));

    // all occurrences
    int count;
    int *indices = str_indices_of(&s, &sub1, &count);
    printf("    All \"ab\" indices in \"%s\": ", s.text);
    for (int i = 0; i < count; i++) {
        printf("%d ", indices[i]);
    }
    printf("(expected: 0 2 4)\n");
    free(indices);

    // all occurrences for substring not found
    indices = str_indices_of(&s, &sub3, &count);
    printf("    All 'xyz' indices: %p (expected NULL)\n", indices);

    str_free(&s, &sub1, &sub2, &sub3);
}

void test_insert(void) {
    printf("\n=== Insert tests ===\n");

    String s = str("Hello world");
    String sub1 = str(", cruel");
    String sub2 = str("!!!");
    String sub3 = str("Start: ");

    print_string(&s, "s");
    print_string(&sub1, "sub1");
    print_string(&sub2, "sub2");
    print_string(&sub3, "sub3");

    // Insert in the middle
    str_insert(&s, &sub1, 5);
    printf("\n    After inserting \"%s\" at index 5: \"%s\" (expected \"Hello, cruel world\")\n", sub1.text, s.text);

    // Insert at end
    str_insert(&s, &sub2, s.len);
    printf("    After inserting \"%s\" at end: \"%s\" (expected \"Hello, cruel world!!!\")\n", sub2.text, s.text);

    // Insert at beginning
    str_insert(&s, &sub3, 0);
    printf("    After inserting \"%s\" at start: \"%s\" (expected \"Start: Hello, cruel world!!!\")\n", sub3.text, s.text);

    // Edge case: empty string
    String empty = str("");
    str_insert(&empty, &sub2, 0);
    printf("    Insert into empty string: \"%s\" (expected \"!!!\")\n", empty.text);

    str_free(&s, &sub1, &sub2, &sub3, &empty);
}

void test_replace(void) {
    printf("\n=== Replace tests ===\n");

    String s1 = str("the quick brown fox jumps over the lazy fox");
    String old1 = str("fox");
    String new1 = str("dog");

    print_string(&s1, "s1");
    print_string(&old1, "old1");
    print_string(&new1, "new1");

    // first occurrence
    str_replace(&s1, &old1, &new1, "first");
    printf("    str_replace(&s1, &old1, &new1, \"first\") sets s1 to: \"%s\"\n", s1.text);

    // last occurrence
    str_replace(&s1, &old1, &new1, "last");
    printf("    str_replace(&s1, &old1, &new1, \"last\") sets s1 to: \"%s\"\n", s1.text);

    str_free(&s1, &old1, &new1);

    // all occurrences
    String s2 = str("abc abc abc");
    String old2 = str("abc");
    String new2 = str("xyz");
    print_string(&s2, "s2");
    print_string(&old2, "old2");
    print_string(&new2, "new2");
    str_replace(&s2, &old2, &new2, "all");
    printf("    str_replace(&s2, &old2, &new2, \"all\") sets s2 to: \"%s\"\n", s2.text);

    str_free(&s2, &old2, &new2);
}

void test_split(void) {
    printf("\n=== Split tests ===\n");

    String s = str("one,two,three,four");
    size_t count;
    String *parts = str_split(&s, ',', &count);

    printf("    Split \"%s\" by ',' returns %zu Strings:\n", s.text, count);
    for (size_t i = 0; i < count; i++) {
        print_string(&parts[i], "part");
        str_free(&parts[i]); // free each substring
    }
    free(parts); // free array

    str_free(&s);
}

void test_str_equals(void) {
    printf("\n=== str_equals tests ===\n");

    String a = str("hello");
    String b = str("hello");
    String c = str("world");
    String d = str("hello!");
    String empty1 = str("");
    String empty2 = str("");

    printf("    \"%s\" equals \"%s\": %d (expected 1)\n",
           a.text, b.text, str_equals(&a, &b));

    printf("    \"%s\" equals \"%s\": %d (expected 0)\n",
           a.text, c.text, str_equals(&a, &c));

    printf("    \"%s\" equals \"%s\": %d (expected 0)\n",
           a.text, d.text, str_equals(&a, &d));

    printf("    \"%s\" equals \"%s\": %d (expected 1)\n",
           empty1.text, empty2.text, str_equals(&empty1, &empty2));

    str_free(&a, &b, &c, &d, &empty1, &empty2);
}

void test_str_slice(void) {
    printf("\n=== str_slice tests ===\n");

    String s = str("Hello, world!");

    String a = str_slice(&s, 0, 5);
    printf("    \"%s\" slice(0,5): \"%s\" (expected \"Hello\")\n",
           s.text, a.text);

    String b = str_slice(&s, 7, 12);
    printf("    \"%s\" slice(7,12): \"%s\" (expected \"world\")\n",
           s.text, b.text);

    String c = str_slice(&s, 0, 100);
    printf("    \"%s\" slice(0,100): \"%s\" (expected \"Hello, world!\")\n",
           s.text, c.text);

    String d = str_slice(&s, 5, 5);
    printf("    \"%s\" slice(5,5): \"%s\" (expected \"\")\n",
           s.text, d.text);

    String e = str_slice(&s, 50, 60);
    printf("    \"%s\" slice(50,60): \"%s\" (expected \"\")\n",
           s.text, e.text);

    str_free(&s, &a, &b, &c, &d, &e);
}

void test_str_repeat(void) {
    printf("\n=== str_repeat tests ===\n");

    String s = str("ab");
    String r1 = str_repeat(&s, 0);
    printf("    \"%s\" repeat 0: \"%s\" (expected \"\")\n", s.text, r1.text);

    String r2 = str_repeat(&s, 1);
    printf("    \"%s\" repeat 1: \"%s\" (expected \"ab\")\n", s.text, r2.text);

    String r3 = str_repeat(&s, 5);
    printf("    \"%s\" repeat 5: \"%s\" (expected \"ababababab\")\n", s.text, r3.text);

    str_free(&s, &r1, &r2, &r3);
}

void test_str_count(void) {
    printf("\n=== str_count tests ===\n");

    String s = str("abababacaca");

    String sub1 = str("ab");
    String sub2 = str("a");
    String sub3 = str("abc");
    String sub4 = str("x");

    printf("    \"%s\" count \"%s\": %zu (expected 3)\n", s.text, sub1.text, str_count(&s, &sub1));
    printf("    \"%s\" count \"%s\": %zu (expected 7)\n", s.text, sub2.text, str_count(&s, &sub2));
    printf("    \"%s\" count \"%s\": %zu (expected 1)\n", s.text, sub3.text, str_count(&s, &sub3));
    printf("    \"%s\" count \"%s\": %zu (expected 0)\n", s.text, sub4.text, str_count(&s, &sub4));

    str_free(&s, &sub1, &sub2, &sub3, &sub4);

}

void test_str_remove(void) {
    printf("\n=== str_remove tests ===\n");

    String s;

    s = str("Hello, world!");
    str_remove(&s, 5, 2); // remove ", "
    printf("    \"%s\" after remove(5,2): \"%s\" (expected \"Helloworld!\")\n", "Hello, world!", s.text);
    str_free(&s);

    s = str("Hello world!");
    str_remove(&s, 0, 6); // remove "Hello "
    printf("    \"%s\" after remove(0,6): \"%s\" (expected \"world!\")\n", "Hello world!", s.text);
    str_free(&s);

    s = str("world!");
    str_remove(&s, 3, 10); // remove past end
    printf("    \"%s\" after remove(3,10): \"%s\" (expected \"wor\")\n", "world!", s.text);
    str_free(&s);

    s = str("wor");
    str_remove(&s, 5, 2); // remove beyond string length → nothing happens
    printf("    \"%s\" after remove(5,2): \"%s\" (expected \"wor\")\n", "wor", s.text);
    str_free(&s);
}

void test_str_trim(void) {
    printf("\n=== str_trim tests ===\n");

    String s1 = str("   Hello world!  ");
    str_trim(&s1);
    printf("    \"   Hello world!  \" -> \"%s\" (expected \"Hello world!\")\n", s1.text);

    String s2 = str("   Leading");
    str_trim_left(&s2);
    printf("    \"   Leading\" -> \"%s\" (expected \"Leading\")\n", s2.text);

    String s3 = str("Trailing   ");
    str_trim_right(&s3);
    printf("    \"Trailing   \" -> \"%s\" (expected \"Trailing\")\n", s3.text);

    String s4 = str("   Both sides   ");
    str_trim_left(&s4);
    str_trim_right(&s4);
    printf("    \"   Both sides   \" -> \"%s\" (expected \"Both sides\")\n", s4.text);

    String s5 = str("      "); // all spaces
    str_trim(&s5);
    printf("    \"      \" -> \"%s\" (expected \"\")\n", s5.text);

    str_free(&s1, &s2, &s3, &s4, &s5);
}

int main(void) {
    printf("====== String Utils Tests ======\n");

    test_str_init();
    test_str_clone();
    test_append();
    test_prepend();
    test_concat();
    test_is_empty();
    test_starts_and_ends_with();
    test_to_upper_to_lower();
    test_contains();
    test_index_functions();
    test_insert();
    test_replace();
    test_split();
    test_str_equals();
    test_str_slice();
    test_str_repeat();
    test_str_count();
    test_str_remove();
    test_str_trim();

    printf("\n====== All tests completed =====\n");
    return 0;
}

