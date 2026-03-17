// test.c
#include "string_util.h"
#include <string.h> // for strcmp used in ASSERT_STR_EQ macro
#include <stdio.h>  // for printf
#include <assert.h> // for assert (optional - can remove if you prefer)

//////////////// test helper functions ///////////////

int all_passed_tests = 0;
int all_failed_tests = 0;

#define ASSERT_TEXT_EQ(label, actual, expected, details, verbose)             \
    do {                                                                       \
        bool test_passed = strcmp((actual), (expected)) == 0;                  \
        if (test_passed) {                                                     \
            printf("    ✅ PASS: %s\n", (label));                              \
            passed++;                                                          \
        } else {                                                               \
            printf("    ❌ FAIL: %s (%s:%d)\n", (label), __FILE__, __LINE__);  \
            failed++;                                                          \
        }                                                                      \
        if (!test_passed || verbose) {                                         \
            printf("        expected: \"%s\"\n", (expected));                  \
            printf("        actual:   \"%s\"\n", (actual));                    \
            printf("        details:\n%s\n", (details));                       \
        }                                                                      \
    } while (0)

#define ASSERT_STR_EQ(label, actual, expected, details, verbose) \
    ASSERT_TEXT_EQ(label, (actual)->text, (expected), details, verbose)

// #define ASSERT_STR_EQ(label, actual, expected, details, verbose)              \
//     do {                                                                      \
//         bool test_passed = strcmp((actual)->text, (expected)) == 0;           \
//         if (test_passed) {                                                    \
//             printf("    ✅ PASS: %s\n", (label));                             \
//             passed++;                                                         \
//         } else {                                                              \
//             printf("    ❌ FAIL: %s (%s:%d)\n", (label), __FILE__, __LINE__); \
//             failed++;                                                         \
//         }                                                                     \
//         if (!test_passed || verbose) {                                        \
//             printf("        expected: \"%s\"\n", (expected));                 \
//             printf("        actual:   \"%s\"\n", (actual)->text);             \
//             printf("        details:\n%s\n", (details));                      \
//         }                                                                     \
//     } while (0)

#define ASSERT_BOOL_TRUE(label, test_passed, details, verbose)                \
    do {                                                                      \
        bool _tp = (bool)(test_passed);                                       \
        if (_tp) {                                                            \
            printf("    ✅ PASS: %s\n", (label));                            \
            passed++;                                                         \
        } else {                                                              \
            printf("    ❌ FAIL: %s (%s:%d)\n", (label), __FILE__, __LINE__);\
            failed++;                                                         \
        }                                                                     \
        if (!_tp || verbose) {                                                \
            printf("        details:\n%s\n", (details));                      \
        }                                                                     \
    } while (0)

#define ASSERT_INT_EQ(label, actual, expected, details, verbose)              \
    do {                                                                      \
        bool test_passed = (actual) == (expected);                            \
        if (test_passed) {                                                    \
            printf("    ✅ PASS: %s\n", (label));                             \
            passed++;                                                         \
        } else {                                                              \
            printf("    ❌ FAIL: %s (%s:%d)\n", (label), __FILE__, __LINE__); \
            printf("        expected: %d\n", (int)(expected));                \
            printf("        actual:   %d\n", (int)(actual));                  \
            failed++;                                                         \
        }                                                                     \
        if (!test_passed || verbose) {                                        \
            printf("        details:\n%s\n", (details));                      \
        }                                                                     \
    } while (0)

static void append_formatted_text(const char *text, char *out, size_t out_cap) {
    if (!text || !out) return;
    size_t offset = strlen(out);
    snprintf(out + offset, out_cap - offset, "            %s", text);
}

static void append_string_details(const char *label, const String *s, char *out, size_t out_cap) {
    if (!s || !s->text || !out) return;
    char s_info[1024]; str_info(s, s_info, sizeof(s_info));
    size_t offset = strlen(out);
    snprintf(out + offset, out_cap - offset, "            String %s: %s\n", label, s_info);
}

////////////////// test functions //////////////////

void test_str_init(bool verbose) {
    printf("\n=== test: str_init() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Simple strings
    append_formatted_text("initialize simple strings\n\n", test_details, sizeof(test_details));
    String *a = str("Hello");
    String *b = str(" world!");
    String *empty = str("");
    append_string_details("a", a, test_details, sizeof(test_details));
    append_string_details("b", b, test_details, sizeof(test_details));
    append_string_details("empty", empty, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 1: simple string", a, "Hello", test_details, verbose);
    ASSERT_STR_EQ("Test 2: simple string with space", b, " world!", test_details, verbose);
    ASSERT_STR_EQ("Test 3: empty string", empty, "", test_details, verbose);
    str_free(&a, &b, &empty);

    // Formatted strings
    test_details[0] = '\0';
    append_formatted_text("initialize formatted strings\n\n", test_details, sizeof(test_details));
    char buf1[128], buf2[128], buf3[128];
    String *f1 = str(fmt(buf1, "Hi my name is %s.", "Luke"));
    String *f2 = str(fmt(buf1, "nested fmt() calls %s", fmt(buf2, "require separate %s", "buffers")));
    String *f3 = str(fmt(buf1, "so do %s %s", fmt(buf2, "%s", "neighboring"), fmt(buf3, "%s", "fmt() calls")));
    append_string_details("f1", f1, test_details, sizeof(test_details));
    append_string_details("f2", f2, test_details, sizeof(test_details));
    append_string_details("f3", f3, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 4: formatted string, simple", f1, "Hi my name is Luke.", test_details, verbose);
    ASSERT_STR_EQ("Test 5: formatted string, nested", f2, "nested fmt() calls require separate buffers", test_details, verbose);
    ASSERT_STR_EQ("Test 6: formatted string, neighboring child fmt() calls", f3, "so do neighboring fmt() calls", test_details, verbose);
    str_free(&f1, &f2, &f3);

    printf("\n    %s test: str_init() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_str_clone(bool verbose) {
    printf("\n=== test: str_clone() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";
    append_formatted_text("clone a string and verify independence after mutation\n\n", test_details, sizeof(test_details));

    String *s1 = str("東京。hello 大阪。world京都");
    String *s2 = str_clone(s1);
    append_string_details("s1", s1, test_details, sizeof(test_details));
    append_string_details("s2", s2, test_details, sizeof(test_details));

    String *sub = str(", naïve ");
    str_insert(s1, sub, 10);

    ASSERT_STR_EQ("Test 1: clone is independent after mutation", s2, "東京。hello 大阪。world京都", test_details, verbose);

    printf("\n    %s test: str_clone() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;

    str_free(&s1, &s2, &sub);
}

void test_append(bool verbose) {
    printf("\n=== test: str_append() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Append to non-empty string
    append_formatted_text("append to a non-empty string\n\n", test_details, sizeof(test_details));
    String *base = str("Hello");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_append(base, ", how are you?");
    ASSERT_STR_EQ("Test 1: append to non-empty string", base, "Hello, how are you?", test_details, verbose);
    str_free(&base);

    // Append UTF-8 to ASCII
    test_details[0] = '\0';
    append_formatted_text("append utf-8 to ascii string\n\n", test_details, sizeof(test_details));
    base = str("Tokyo: ");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_append(base, "東京 🗼");
    ASSERT_STR_EQ("Test 2: append utf-8 to ascii", base, "Tokyo: 東京 🗼", test_details, verbose);
    str_free(&base);

    // Append ASCII to UTF-8
    test_details[0] = '\0';
    append_formatted_text("append ascii to utf-8 string\n\n", test_details, sizeof(test_details));
    base = str("café ");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_append(base, "au lait");
    ASSERT_STR_EQ("Test 3: append ascii to utf-8", base, "café au lait", test_details, verbose);
    str_free(&base);

    // Append emoji to emoji
    test_details[0] = '\0';
    append_formatted_text("append emoji to emoji string\n\n", test_details, sizeof(test_details));
    base = str("🍎🍊");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_append(base, "🍋🍇");
    ASSERT_STR_EQ("Test 4: append emoji to emoji", base, "🍎🍊🍋🍇", test_details, verbose);
    str_free(&base);

    // Append mixed multi-byte: Greek, Korean, symbols
    test_details[0] = '\0';
    append_formatted_text("append mixed multi-byte characters\n\n", test_details, sizeof(test_details));
    base = str("αβγ → ");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_append(base, "한국어 ★");
    ASSERT_STR_EQ("Test 5: append mixed multi-byte", base, "αβγ → 한국어 ★", test_details, verbose);
    str_free(&base);

    // Append to empty string
    test_details[0] = '\0';
    append_formatted_text("append to an empty string\n\n", test_details, sizeof(test_details));
    String *empty = str("");
    append_string_details("empty", empty, test_details, sizeof(test_details));
    str_append(empty, "something");
    ASSERT_STR_EQ("Test 6: append to empty string", empty, "something", test_details, verbose);
    str_free(&empty);

    // Append empty string to non-empty (no-op)
    test_details[0] = '\0';
    append_formatted_text("append empty string (no-op)\n\n", test_details, sizeof(test_details));
    base = str("unchanged");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_append(base, "");
    ASSERT_STR_EQ("Test 7: append empty string is no-op", base, "unchanged", test_details, verbose);
    str_free(&base);

    // Append to NULL (should return -1)
    test_details[0] = '\0';
    append_formatted_text("append to NULL (should return -1)\n\n", test_details, sizeof(test_details));
    int res = str_append(NULL, "something");
    if (res == -1) {
        printf("    ✅ PASS: Test 8: append(NULL, ...) returned -1\n");
        passed++;
    } else {
        printf("    ❌ FAIL: Test 8: append(NULL, ...) returned %d (expected -1)\n", res);
        failed++;
    }

    printf("\n    %s test: str_append() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_prepend(bool verbose) {
    printf("\n=== test: str_prepend() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Prepend to non-empty string
    append_formatted_text("prepend to a non-empty string\n\n", test_details, sizeof(test_details));
    String *base = str("world!");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_prepend("Hello, ", base);
    ASSERT_STR_EQ("Test 1: prepend to non-empty string", base, "Hello, world!", test_details, verbose);
    str_free(&base);

    // Prepend UTF-8 to ASCII
    test_details[0] = '\0';
    append_formatted_text("prepend utf-8 to ascii string\n\n", test_details, sizeof(test_details));
    base = str(" is beautiful");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_prepend("東京", base);
    ASSERT_STR_EQ("Test 2: prepend utf-8 to ascii", base, "東京 is beautiful", test_details, verbose);
    str_free(&base);

    // Prepend ASCII to UTF-8
    test_details[0] = '\0';
    append_formatted_text("prepend ascii to utf-8 string\n\n", test_details, sizeof(test_details));
    base = str("naïve 🌸");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_prepend("so ", base);
    ASSERT_STR_EQ("Test 3: prepend ascii to utf-8", base, "so naïve 🌸", test_details, verbose);
    str_free(&base);

    // Prepend emoji to emoji
    test_details[0] = '\0';
    append_formatted_text("prepend emoji to emoji string\n\n", test_details, sizeof(test_details));
    base = str("🌍🌎");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_prepend("🌏", base);
    ASSERT_STR_EQ("Test 4: prepend emoji to emoji", base, "🌏🌍🌎", test_details, verbose);
    str_free(&base);

    // Prepend mixed multi-byte: symbols, CJK, accented
    test_details[0] = '\0';
    append_formatted_text("prepend mixed multi-byte characters\n\n", test_details, sizeof(test_details));
    base = str("★ five stars");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_prepend("©2024 ∞ ", base);
    ASSERT_STR_EQ("Test 5: prepend mixed multi-byte", base, "©2024 ∞ ★ five stars", test_details, verbose);
    str_free(&base);

    // Prepend to empty string
    test_details[0] = '\0';
    append_formatted_text("prepend to an empty string\n\n", test_details, sizeof(test_details));
    String *empty = str("");
    append_string_details("empty", empty, test_details, sizeof(test_details));
    str_prepend("something", empty);
    ASSERT_STR_EQ("Test 6: prepend to empty string", empty, "something", test_details, verbose);
    str_free(&empty);

    // Prepend empty string to non-empty (no-op)
    test_details[0] = '\0';
    append_formatted_text("prepend empty string (no-op)\n\n", test_details, sizeof(test_details));
    base = str("unchanged");
    append_string_details("base", base, test_details, sizeof(test_details));
    str_prepend("", base);
    ASSERT_STR_EQ("Test 7: prepend empty string is no-op", base, "unchanged", test_details, verbose);
    str_free(&base);

    // Prepend to NULL (should return -1)
    test_details[0] = '\0';
    append_formatted_text("prepend to NULL (should return -1)\n\n", test_details, sizeof(test_details));
    int res = str_prepend("something", NULL);
    if (res == -1) {
        printf("    ✅ PASS: Test 8: prepend(..., NULL) returned -1\n");
        passed++;
    } else {
        printf("    ❌ FAIL: Test 8: prepend(..., NULL) returned %d (expected -1)\n", res);
        failed++;
    }

    printf("\n    %s test: str_prepend() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_concat(bool verbose) {
    printf("\n=== test: str_concat() ===\n");
    int passed = 0, failed = 0;

    // Test 1: classic concat into first (output_index=0)
    // Mix of: ASCII, emoji (4-byte), CJK (3-byte), Latin accented (2-byte)
    char test_details[1024] = "";
    append_formatted_text("concat all strings' text into first string in list\n\n", test_details, sizeof(test_details));
    String *a = str("Hello 😀");
    String *b = str(", beautiful 東京。大阪");
    String *c = str("world ¡café!");
    String *d = str(" naïve");
    append_string_details("a", a, test_details, sizeof(test_details));
    append_string_details("b", b, test_details, sizeof(test_details));
    append_string_details("c", c, test_details, sizeof(test_details));
    append_string_details("d", d, test_details, sizeof(test_details));
    String *parts1[] = {a, b, c, d};
    str_concat(parts1);
    char *expected_text = "Hello 😀, beautiful 東京。大阪world ¡café! naïve";
    ASSERT_STR_EQ("Test 1: classic concat into first", a, expected_text, test_details, verbose);
    str_free(&a, &b, &c, &d);

    // Test 2: output_index in the middle
    // Verifies snapshot logic: output string's original text must appear at index 2
    test_details[0] = '\0';
    append_formatted_text("concat all strings' text into 3rd string in list\n\n", test_details, sizeof(test_details));
    a = str("α");        // Greek (2-byte)
    b = str("β");
    c = str("γ→δ");      // arrow symbol (3-byte)
    d = str("ε");
    append_string_details("a", a, test_details, sizeof(test_details));
    append_string_details("b", b, test_details, sizeof(test_details));
    append_string_details("c", c, test_details, sizeof(test_details));
    append_string_details("d", d, test_details, sizeof(test_details));
    String *parts2[] = {a, b, c, d};
    str_concat(parts2, .output_index=2);
    expected_text = "αβγ→δε";
    ASSERT_STR_EQ("Test 2: output_index=2 (middle)", c, expected_text, test_details, verbose);
    str_free(&a, &b, &c, &d);

    // Test 3: output_index at the last position
    // Verifies snapshot + separator when output is at the end
    test_details[0] = '\0';
    append_formatted_text("concat with separator, output into last string in list\n\n", test_details, sizeof(test_details));
    a = str("苹果");      // CJK (3-byte each)
    b = str("バナナ");    // Katakana (3-byte each)
    c = str("체리");      // Korean (3-byte each)
    d = str("🍓");        // emoji (4-byte)
    String *sep3 = str(" · ");  // middle dot (3-byte)
    append_string_details("a", a, test_details, sizeof(test_details));
    append_string_details("b", b, test_details, sizeof(test_details));
    append_string_details("c", c, test_details, sizeof(test_details));
    append_string_details("d", d, test_details, sizeof(test_details));
    append_string_details("sep", sep3, test_details, sizeof(test_details));
    String *parts3[] = {a, b, c, d};
    str_concat(parts3, .output_index=3, .sep=sep3);
    expected_text = "苹果 · バナナ · 체리 · 🍓";
    ASSERT_STR_EQ("Test 3: output_index=3 (last) with separator", d, expected_text, test_details, verbose);
    str_free(&a, &b, &c, &d, &sep3);

    // Test 4: separator only, output_index=0
    // Mix of symbols, RTL-adjacent chars, and multi-byte
    test_details[0] = '\0';
    append_formatted_text("concat with separator and symbol-heavy strings\n\n", test_details, sizeof(test_details));
    a = str("©2024");
    b = str("™ brand");
    c = str("∞ possibilities");
    d = str("★ five stars");
    String *sep4 = str(" ∥ ");  // double vertical line (3-byte)
    append_string_details("a", a, test_details, sizeof(test_details));
    append_string_details("b", b, test_details, sizeof(test_details));
    append_string_details("c", c, test_details, sizeof(test_details));
    append_string_details("d", d, test_details, sizeof(test_details));
    append_string_details("sep", sep4, test_details, sizeof(test_details));
    String *parts4[] = {a, b, c, d};
    str_concat(parts4, .sep=sep4);
    expected_text = "©2024 ∥ ™ brand ∥ ∞ possibilities ∥ ★ five stars";
    ASSERT_STR_EQ("Test 4: separator with symbols, output_index=0", a, expected_text, test_details, verbose);
    str_free(&a, &b, &c, &d, &sep4);
    
    // Test 5: two strings, output_index=1
    // Minimal case for snapshot path with only two strings
    test_details[0] = '\0';
    append_formatted_text("concat two strings, output into second\n\n", test_details, sizeof(test_details));
    a = str("前");   // CJK (3-byte)
    b = str("後");
    append_string_details("a", a, test_details, sizeof(test_details));
    append_string_details("b", b, test_details, sizeof(test_details));
    String *parts5[] = {a, b};
    str_concat(parts5, .output_index=1);
    expected_text = "前後";
    ASSERT_STR_EQ("Test 5: two strings, output_index=1", b, expected_text, test_details, verbose);
    str_free(&a, &b);

    // Test 6: single string (no-op)
    test_details[0] = '\0';
    append_formatted_text("concat a single string (should remain unchanged)\n\n", test_details, sizeof(test_details));
    String *single = str("alone 🌍");
    append_string_details("single", single, test_details, sizeof(test_details));
    String *one[] = {single};
    str_concat(one);
    expected_text = "alone 🌍";
    ASSERT_STR_EQ("Test 6: single string, no change", single, expected_text, test_details, verbose);
    str_free(&single);

    // Test 7: empty string at output_index
    // tmp will be NULL, verifies the tmp guard in the write loop
    test_details[0] = '\0';
    append_formatted_text("concat with empty string at output_index=0\n\n", test_details, sizeof(test_details));
    a = str("");
    b = str("hello 🌸");
    c = str(" world");
    append_string_details("a", a, test_details, sizeof(test_details));
    append_string_details("b", b, test_details, sizeof(test_details));
    append_string_details("c", c, test_details, sizeof(test_details));
    String *parts7[] = {a, b, c};
    str_concat(parts7);
    expected_text = "hello 🌸 world";
    ASSERT_STR_EQ("Test 7: empty string at output_index=0", a, expected_text, test_details, verbose);
    str_free(&a, &b, &c);

    printf("\n    %s test: str_concat() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;

}

void test_to_upper_to_lower(bool verbose) {
    printf("\n=== test: str_to_upper(), str_to_lower() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Basic upper/lower
    append_formatted_text("convert string to upper then lower\n\n", test_details, sizeof(test_details));
    String *s1 = str("Hello World!");
    append_string_details("s1", s1, test_details, sizeof(test_details));
    str_to_upper(s1);
    ASSERT_STR_EQ("Test 1: to_upper", s1, "HELLO WORLD!", test_details, verbose);
    str_to_lower(s1);
    ASSERT_STR_EQ("Test 2: to_lower", s1, "hello world!", test_details, verbose);
    str_free(&s1);

    // Mixed characters and numbers
    test_details[0] = '\0';
    append_formatted_text("convert mixed alphanumeric string\n\n", test_details, sizeof(test_details));
    String *mixed = str("123 AbC! xyZ");
    append_string_details("mixed", mixed, test_details, sizeof(test_details));
    str_to_upper(mixed);
    ASSERT_STR_EQ("Test 3: to_upper mixed alphanumeric", mixed, "123 ABC! XYZ", test_details, verbose);
    str_to_lower(mixed);
    ASSERT_STR_EQ("Test 4: to_lower mixed alphanumeric", mixed, "123 abc! xyz", test_details, verbose);
    str_free(&mixed);

    // UTF-8 accented characters
    test_details[0] = '\0';
    append_formatted_text("convert accented and multi-byte characters\n\n", test_details, sizeof(test_details));
    String *accented = str("café NAÏVE résumé");
    append_string_details("accented", accented, test_details, sizeof(test_details));
    str_to_upper(accented);
    ASSERT_STR_EQ("Test 5: to_upper accented", accented, "CAFÉ NAÏVE RÉSUMÉ", test_details, verbose);
    str_to_lower(accented);
    ASSERT_STR_EQ("Test 6: to_lower accented", accented, "café naïve résumé", test_details, verbose);
    str_free(&accented);

    // Greek characters
    test_details[0] = '\0';
    append_formatted_text("convert greek characters\n\n", test_details, sizeof(test_details));
    String *greek = str("αβγ ΑΒΓ");
    append_string_details("greek", greek, test_details, sizeof(test_details));
    str_to_upper(greek);
    ASSERT_STR_EQ("Test 7: to_upper greek", greek, "ΑΒΓ ΑΒΓ", test_details, verbose);
    str_to_lower(greek);
    ASSERT_STR_EQ("Test 8: to_lower greek", greek, "αβγ αβγ", test_details, verbose);
    str_free(&greek);

    // Symbols and emoji (should remain unchanged)
    test_details[0] = '\0';
    append_formatted_text("symbols and emoji should remain unchanged\n\n", test_details, sizeof(test_details));
    String *symbols = str("★ © 😀 ∞ 🍎");
    append_string_details("symbols", symbols, test_details, sizeof(test_details));
    str_to_upper(symbols);
    ASSERT_STR_EQ("Test 9: to_upper symbols unchanged", symbols, "★ © 😀 ∞ 🍎", test_details, verbose);
    str_to_lower(symbols);
    ASSERT_STR_EQ("Test 10: to_lower symbols unchanged", symbols, "★ © 😀 ∞ 🍎", test_details, verbose);
    str_free(&symbols);

    // Empty string
    test_details[0] = '\0';
    append_formatted_text("convert empty string (no-op)\n\n", test_details, sizeof(test_details));
    String *empty = str("");
    append_string_details("empty", empty, test_details, sizeof(test_details));
    str_to_upper(empty);
    ASSERT_STR_EQ("Test 11: to_upper empty string", empty, "", test_details, verbose);
    str_to_lower(empty);
    ASSERT_STR_EQ("Test 12: to_lower empty string", empty, "", test_details, verbose);
    str_free(&empty);

    printf("\n    %s test: str_to_upper(), str_to_lower() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_insert(bool verbose) {
    printf("\n=== test: str_insert() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Insert in the middle
    append_formatted_text("insert substring in the middle\n\n", test_details, sizeof(test_details));
    String *s    = str("Hello world");
    String *sub1 = str(", cruel 京都");
    append_string_details("s", s, test_details, sizeof(test_details));
    append_string_details("sub1", sub1, test_details, sizeof(test_details));
    str_insert(s, sub1, 5);
    ASSERT_STR_EQ("Test 1: insert in middle", s, "Hello, cruel 京都 world", test_details, verbose);

    // Insert at end
    test_details[0] = '\0';
    append_formatted_text("insert substring at end\n\n", test_details, sizeof(test_details));
    String *sub2 = str("😀!!!😀");
    append_string_details("s", s, test_details, sizeof(test_details));
    append_string_details("sub2", sub2, test_details, sizeof(test_details));
    str_insert(s, sub2, s->len);
    ASSERT_STR_EQ("Test 2: insert at end", s, "Hello, cruel 京都 world😀!!!😀", test_details, verbose);

    // Insert at beginning
    test_details[0] = '\0';
    append_formatted_text("insert substring at beginning\n\n", test_details, sizeof(test_details));
    String *sub3 = str("Start: résumé");
    append_string_details("s", s, test_details, sizeof(test_details));
    append_string_details("sub3", sub3, test_details, sizeof(test_details));
    str_insert(s, sub3, 0);
    ASSERT_STR_EQ("Test 3: insert at beginning", s, "Start: résuméHello, cruel 京都 world😀!!!😀", test_details, verbose);
    str_free(&s, &sub1, &sub2, &sub3);

    // Insert into empty string
    test_details[0] = '\0';
    append_formatted_text("insert into empty string\n\n", test_details, sizeof(test_details));
    String *empty = str("");
    String *sub4  = str("😀!!!😀");
    append_string_details("empty", empty, test_details, sizeof(test_details));
    append_string_details("sub4", sub4, test_details, sizeof(test_details));
    str_insert(empty, sub4, 0);
    ASSERT_STR_EQ("Test 4: insert into empty string", empty, "😀!!!😀", test_details, verbose);
    str_free(&empty, &sub4);

    // Insert multi-byte into multi-byte
    test_details[0] = '\0';
    append_formatted_text("insert multi-byte into multi-byte string\n\n", test_details, sizeof(test_details));
    String *cjk  = str("東京都");
    String *sub5 = str("★大阪★");
    append_string_details("cjk", cjk, test_details, sizeof(test_details));
    append_string_details("sub5", sub5, test_details, sizeof(test_details));
    str_insert(cjk, sub5, 2);
    ASSERT_STR_EQ("Test 5: insert multi-byte into multi-byte", cjk, "東京★大阪★都", test_details, verbose);
    str_free(&cjk, &sub5);

    printf("\n    %s test: str_insert() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_replace(bool verbose) {
    printf("\n=== test: str_replace() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Replace first occurrence
    append_formatted_text("replace first occurrence\n\n", test_details, sizeof(test_details));
    String *s1   = str("the quick brown fox jumps over the lazy fox");
    String *old1 = str("fox");
    String *new1 = str("😀");
    append_string_details("s1",   s1,   test_details, sizeof(test_details));
    append_string_details("old1", old1, test_details, sizeof(test_details));
    append_string_details("new1", new1, test_details, sizeof(test_details));
    str_replace(s1, old1, new1, "first");
    ASSERT_STR_EQ("Test 1: replace first occurrence", s1, "the quick brown 😀 jumps over the lazy fox", test_details, verbose);

    // Replace last occurrence
    test_details[0] = '\0';
    append_formatted_text("replace last occurrence\n\n", test_details, sizeof(test_details));
    append_string_details("s1", s1, test_details, sizeof(test_details));
    str_replace(s1, old1, new1, "last");
    ASSERT_STR_EQ("Test 2: replace last occurrence", s1, "the quick brown 😀 jumps over the lazy 😀", test_details, verbose);
    str_free(&s1, &old1, &new1);

    // Replace all occurrences
    test_details[0] = '\0';
    append_formatted_text("replace all occurrences\n\n", test_details, sizeof(test_details));
    String *s2   = str("abc abc abc");
    String *old2 = str("abc");
    String *new2 = str("xyz");
    append_string_details("s2",   s2,   test_details, sizeof(test_details));
    append_string_details("old2", old2, test_details, sizeof(test_details));
    append_string_details("new2", new2, test_details, sizeof(test_details));
    str_replace(s2, old2, new2, "all");
    ASSERT_STR_EQ("Test 3: replace all occurrences", s2, "xyz xyz xyz", test_details, verbose);
    str_free(&s2, &old2, &new2);

    // Replace all with overlapping pattern
    test_details[0] = '\0';
    append_formatted_text("replace all with overlapping pattern\n\n", test_details, sizeof(test_details));
    String *s3   = str("hahahahahahahahaha");
    String *old3 = str("hahahaha");
    String *new3 = str("ha😀");
    append_string_details("s3",   s3,   test_details, sizeof(test_details));
    append_string_details("old3", old3, test_details, sizeof(test_details));
    append_string_details("new3", new3, test_details, sizeof(test_details));
    str_replace(s3, old3, new3, "all");
    ASSERT_STR_EQ("Test 4: replace overlapping pattern", s3, "ha😀ha😀ha", test_details, verbose);
    str_free(&s3, &old3, &new3);

    // Replace UTF-8 with UTF-8
    test_details[0] = '\0';
    append_formatted_text("replace utf-8 substring with utf-8\n\n", test_details, sizeof(test_details));
    String *s4   = str("東京。大阪。京都");
    String *old4 = str("。");
    String *new4 = str(" → ");
    append_string_details("s4",   s4,   test_details, sizeof(test_details));
    append_string_details("old4", old4, test_details, sizeof(test_details));
    append_string_details("new4", new4, test_details, sizeof(test_details));
    str_replace(s4, old4, new4, "all");
    ASSERT_STR_EQ("Test 5: replace utf-8 with utf-8", s4, "東京 → 大阪 → 京都", test_details, verbose);
    str_free(&s4, &old4, &new4);

    // Replace with empty string (deletion)
    test_details[0] = '\0';
    append_formatted_text("replace with empty string (deletion)\n\n", test_details, sizeof(test_details));
    String *s5   = str("hello 😀 world 😀");
    String *old5 = str(" 😀");
    String *new5 = str("");
    append_string_details("s5",   s5,   test_details, sizeof(test_details));
    append_string_details("old5", old5, test_details, sizeof(test_details));
    append_string_details("new5", new5, test_details, sizeof(test_details));
    str_replace(s5, old5, new5, "all");
    ASSERT_STR_EQ("Test 6: replace with empty string", s5, "hello world", test_details, verbose);
    str_free(&s5, &old5, &new5);

    printf("\n    %s test: str_replace() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_str_repeat(bool verbose) {
    printf("\n=== test: str_repeat() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Repeat once (no-op)
    append_formatted_text("repeat string once (no-op)\n\n", test_details, sizeof(test_details));
    String *s = str("ha");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_repeat(s, 1);
    ASSERT_STR_EQ("Test 1: repeat once", s, "ha", test_details, verbose);

    // Repeat multiple times
    test_details[0] = '\0';
    append_formatted_text("repeat string 3 times\n\n", test_details, sizeof(test_details));
    append_string_details("s", s, test_details, sizeof(test_details));
    str_repeat(s, 3);
    ASSERT_STR_EQ("Test 2: repeat 3 times", s, "hahaha", test_details, verbose);

    // Repeat zero times (clear)
    test_details[0] = '\0';
    append_formatted_text("repeat string 0 times (empty result)\n\n", test_details, sizeof(test_details));
    append_string_details("s", s, test_details, sizeof(test_details));
    str_repeat(s, 0);
    ASSERT_STR_EQ("Test 3: repeat 0 times", s, "", test_details, verbose);
    str_free(&s);

    // Repeat UTF-8 string
    test_details[0] = '\0';
    append_formatted_text("repeat utf-8 string\n\n", test_details, sizeof(test_details));
    s = str("東京");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_repeat(s, 3);
    ASSERT_STR_EQ("Test 4: repeat utf-8", s, "東京東京東京", test_details, verbose);
    str_free(&s);

    // Repeat emoji string
    test_details[0] = '\0';
    append_formatted_text("repeat emoji string\n\n", test_details, sizeof(test_details));
    s = str("😀🌍");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_repeat(s, 3);
    ASSERT_STR_EQ("Test 5: repeat emoji", s, "😀🌍😀🌍😀🌍", test_details, verbose);
    str_free(&s);

    // Repeat mixed multi-byte
    test_details[0] = '\0';
    append_formatted_text("repeat mixed multi-byte string\n\n", test_details, sizeof(test_details));
    s = str("★café");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_repeat(s, 2);
    ASSERT_STR_EQ("Test 6: repeat mixed multi-byte", s, "★café★café", test_details, verbose);
    str_free(&s);

    printf("\n    %s test: str_repeat() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_str_remove(bool verbose) {
    printf("\n=== test: str_remove() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Remove from middle
    append_formatted_text("remove substring from middle\n\n", test_details, sizeof(test_details));
    String *s = str("Hello, world!");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_remove(s, 5, 2);
    ASSERT_STR_EQ("Test 1: remove from middle", s, "Helloworld!", test_details, verbose);
    str_free(&s);

    // Remove from start
    test_details[0] = '\0';
    append_formatted_text("remove substring from start\n\n", test_details, sizeof(test_details));
    s = str("Hello world!");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_remove(s, 0, 6);
    ASSERT_STR_EQ("Test 2: remove from start", s, "world!", test_details, verbose);
    str_free(&s);

    // Remove past end (clamp)
    test_details[0] = '\0';
    append_formatted_text("remove past end (should clamp)\n\n", test_details, sizeof(test_details));
    s = str("world!");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_remove(s, 3, 10);
    ASSERT_STR_EQ("Test 3: remove past end clamps", s, "wor", test_details, verbose);
    str_free(&s);

    // Remove beyond length (no-op)
    test_details[0] = '\0';
    append_formatted_text("remove at index beyond length (no-op)\n\n", test_details, sizeof(test_details));
    s = str("wor");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_remove(s, 5, 2);
    ASSERT_STR_EQ("Test 4: remove beyond length is no-op", s, "wor", test_details, verbose);
    str_free(&s);

    // Remove UTF-8 runes from middle
    test_details[0] = '\0';
    append_formatted_text("remove utf-8 runes from middle\n\n", test_details, sizeof(test_details));
    s = str("東京。大阪。京都");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_remove(s, 2, 1); // remove 。
    ASSERT_STR_EQ("Test 5: remove utf-8 from middle", s, "東京大阪。京都", test_details, verbose);
    str_free(&s);

    // Remove emoji
    test_details[0] = '\0';
    append_formatted_text("remove emoji from string\n\n", test_details, sizeof(test_details));
    s = str("hello 😀 world 🌍");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_remove(s, 6, 1); // remove 😀
    ASSERT_STR_EQ("Test 6: remove emoji", s, "hello  world 🌍", test_details, verbose);
    str_free(&s);

    // Remove entire string
    test_details[0] = '\0';
    append_formatted_text("remove entire string contents\n\n", test_details, sizeof(test_details));
    s = str("café");
    append_string_details("s", s, test_details, sizeof(test_details));
    str_remove(s, 0, s->len);
    ASSERT_STR_EQ("Test 7: remove entire string", s, "", test_details, verbose);
    str_free(&s);

    printf("\n    %s test: str_remove() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_str_trim(bool verbose) {
    printf("\n=== test: str_trim(), str_trim_left(), str_trim_right() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Trim both sides
    append_formatted_text("trim whitespace from both sides\n\n", test_details, sizeof(test_details));
    String *s1 = str("   Hello world!  ");
    append_string_details("s1", s1, test_details, sizeof(test_details));
    str_trim(s1);
    ASSERT_STR_EQ("Test 1: trim both sides", s1, "Hello world!", test_details, verbose);
    str_free(&s1);

    // Trim left only
    test_details[0] = '\0';
    append_formatted_text("trim whitespace from left only\n\n", test_details, sizeof(test_details));
    String *s2 = str("   Leading");
    append_string_details("s2", s2, test_details, sizeof(test_details));
    str_trim_left(s2);
    ASSERT_STR_EQ("Test 2: trim left", s2, "Leading", test_details, verbose);
    str_free(&s2);

    // Trim right only
    test_details[0] = '\0';
    append_formatted_text("trim whitespace from right only\n\n", test_details, sizeof(test_details));
    String *s3 = str("Trailing   ");
    append_string_details("s3", s3, test_details, sizeof(test_details));
    str_trim_right(s3);
    ASSERT_STR_EQ("Test 3: trim right", s3, "Trailing", test_details, verbose);
    str_free(&s3);

    // Trim left then right
    test_details[0] = '\0';
    append_formatted_text("trim left then right\n\n", test_details, sizeof(test_details));
    String *s4 = str("   Both sides   ");
    append_string_details("s4", s4, test_details, sizeof(test_details));
    str_trim_left(s4);
    str_trim_right(s4);
    ASSERT_STR_EQ("Test 4: trim left then right", s4, "Both sides", test_details, verbose);
    str_free(&s4);

    // Trim all spaces
    test_details[0] = '\0';
    append_formatted_text("trim string of only spaces\n\n", test_details, sizeof(test_details));
    String *s5 = str("      ");
    append_string_details("s5", s5, test_details, sizeof(test_details));
    str_trim(s5);
    ASSERT_STR_EQ("Test 5: trim all spaces", s5, "", test_details, verbose);
    str_free(&s5);

    // Trim with UTF-8 content (whitespace only at edges)
    test_details[0] = '\0';
    append_formatted_text("trim with utf-8 content\n\n", test_details, sizeof(test_details));
    String *s6 = str("  東京。大阪  ");
    append_string_details("s6", s6, test_details, sizeof(test_details));
    str_trim(s6);
    ASSERT_STR_EQ("Test 6: trim with utf-8 content", s6, "東京。大阪", test_details, verbose);
    str_free(&s6);

    // Trim with emoji content
    test_details[0] = '\0';
    append_formatted_text("trim with emoji content\n\n", test_details, sizeof(test_details));
    String *s7 = str("   😀 café 🌍   ");
    append_string_details("s7", s7, test_details, sizeof(test_details));
    str_trim(s7);
    ASSERT_STR_EQ("Test 7: trim with emoji content", s7, "😀 café 🌍", test_details, verbose);
    str_free(&s7);

    // Trim empty string (no-op)
    test_details[0] = '\0';
    append_formatted_text("trim empty string (no-op)\n\n", test_details, sizeof(test_details));
    String *s8 = str("");
    append_string_details("s8", s8, test_details, sizeof(test_details));
    str_trim(s8);
    ASSERT_STR_EQ("Test 8: trim empty string", s8, "", test_details, verbose);
    str_free(&s8);

    // Trim right with multi-byte trailing whitespace
    test_details[0] = '\0';
    append_formatted_text("trim right with multi-byte trailing runes\n\n", test_details, sizeof(test_details));
    String *s9 = str("café   ");   // accented char followed by ASCII spaces
    append_string_details("s9", s9, test_details, sizeof(test_details));
    str_trim_right(s9);
    ASSERT_STR_EQ("Test 9: trim right multi-byte content", s9, "café", test_details, verbose);
    str_free(&s9);

    // Trim right with multi-byte trailing emoji
    test_details[0] = '\0';
    append_formatted_text("trim right with trailing emoji (4-byte rune)\n\n", test_details, sizeof(test_details));
    String *s10 = str("hello 東京   😀   ");  // spaces after a 4-byte emoji
    append_string_details("s10", s10, test_details, sizeof(test_details));
    str_trim_right(s10);
    ASSERT_STR_EQ("Test 10: trim right trailing emoji then spaces", s10, "hello 東京   😀", test_details, verbose);
    str_free(&s10);

    printf("\n    %s test: str_trim() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_str_equals(bool verbose) {
    printf("\n=== test: str_equals() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Basic equality
    append_formatted_text("compare equal and unequal strings\n\n", test_details, sizeof(test_details));
    String *a = str("hello");
    String *b = str("hello");
    String *c = str("world");
    String *d = str("hello!");
    append_string_details("a", a, test_details, sizeof(test_details));
    append_string_details("b", b, test_details, sizeof(test_details));
    append_string_details("c", c, test_details, sizeof(test_details));
    append_string_details("d", d, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 1: equal strings",          str_equals(a, b) == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 2: different strings",      str_equals(a, c) == 0, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 3: prefix mismatch",        str_equals(a, d) == 0, test_details, verbose);
    str_free(&a, &b, &c, &d);

    // Empty strings
    test_details[0] = '\0';
    append_formatted_text("compare empty strings\n\n", test_details, sizeof(test_details));
    String *empty1 = str("");
    String *empty2 = str("");
    append_string_details("empty1", empty1, test_details, sizeof(test_details));
    append_string_details("empty2", empty2, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 4: empty strings equal", str_equals(empty1, empty2) == 1, test_details, verbose);
    str_free(&empty1, &empty2);

    // UTF-8 equality
    test_details[0] = '\0';
    append_formatted_text("compare utf-8 strings\n\n", test_details, sizeof(test_details));
    String *u1 = str("東京。café 😀");
    String *u2 = str("東京。café 😀");
    String *u3 = str("東京。café 🌍");
    append_string_details("u1", u1, test_details, sizeof(test_details));
    append_string_details("u2", u2, test_details, sizeof(test_details));
    append_string_details("u3", u3, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 5: utf-8 equal",     str_equals(u1, u2) == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 6: utf-8 not equal", str_equals(u1, u3) == 0, test_details, verbose);
    str_free(&u1, &u2, &u3);

    printf("\n    %s test: str_equals() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_is_empty(bool verbose) {
    printf("\n=== test: str_is_empty() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    append_formatted_text("check empty and non-empty strings\n\n", test_details, sizeof(test_details));
    String *empty    = str("");
    String *nonempty = str("Hello 😀");
    append_string_details("empty",    empty,    test_details, sizeof(test_details));
    append_string_details("nonempty", nonempty, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 1: empty string",     str_is_empty(empty)    == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 2: non-empty string", str_is_empty(nonempty) == 0, test_details, verbose);
    str_free(&empty, &nonempty);

    // UTF-8 non-empty
    test_details[0] = '\0';
    append_formatted_text("check utf-8 non-empty string\n\n", test_details, sizeof(test_details));
    String *utf8 = str("東京");
    append_string_details("utf8", utf8, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 3: utf-8 non-empty", str_is_empty(utf8) == 0, test_details, verbose);
    str_free(&utf8);

    // NULL pointer
    test_details[0] = '\0';
    append_formatted_text("check NULL pointer\n\n", test_details, sizeof(test_details));
    String *null_ptr = NULL;
    ASSERT_BOOL_TRUE("Test 4: NULL pointer", str_is_empty(null_ptr) == 1, test_details, verbose);

    printf("\n    %s test: str_is_empty() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_starts_and_ends_with(bool verbose) {
    printf("\n=== test: str_starts_with(), str_ends_with() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Basic starts/ends with
    append_formatted_text("basic starts_with and ends_with\n\n", test_details, sizeof(test_details));
    String *s           = str("hello world");
    String *pre1        = str("hello");
    String *pre2        = str("world");
    String *suf1        = str("world");
    String *suf2        = str("hello");
    String *long_prefix = str("hello world!!!");
    append_string_details("s",           s,           test_details, sizeof(test_details));
    append_string_details("pre1",        pre1,        test_details, sizeof(test_details));
    append_string_details("pre2",        pre2,        test_details, sizeof(test_details));
    append_string_details("suf1",        suf1,        test_details, sizeof(test_details));
    append_string_details("suf2",        suf2,        test_details, sizeof(test_details));
    append_string_details("long_prefix", long_prefix, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 1: starts with prefix",         str_starts_with(s, pre1)        == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 2: does not start with suffix", str_starts_with(s, pre2)        == 0, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 3: ends with suffix",           str_ends_with(s, suf1)          == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 4: does not end with prefix",   str_ends_with(s, suf2)          == 0, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 5: prefix longer than string",  str_starts_with(s, long_prefix) == 0, test_details, verbose);
    str_free(&s, &pre1, &pre2, &suf1, &suf2, &long_prefix);

    // UTF-8 starts/ends with
    test_details[0] = '\0';
    append_formatted_text("utf-8 starts_with and ends_with\n\n", test_details, sizeof(test_details));
    String *u    = str("東京。café 😀");
    String *upre = str("東京");
    String *usuf = str("café 😀");
    String *uno  = str("大阪");
    append_string_details("u",    u,    test_details, sizeof(test_details));
    append_string_details("upre", upre, test_details, sizeof(test_details));
    append_string_details("usuf", usuf, test_details, sizeof(test_details));
    append_string_details("uno",  uno,  test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 6: utf-8 starts with",         str_starts_with(u, upre) == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 7: utf-8 ends with",           str_ends_with(u, usuf)   == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 8: utf-8 does not start with", str_starts_with(u, uno)  == 0, test_details, verbose);
    str_free(&u, &upre, &usuf, &uno);

    // Empty string edge cases
    test_details[0] = '\0';
    append_formatted_text("empty string edge cases\n\n", test_details, sizeof(test_details));
    String *empty = str("");
    append_string_details("empty", empty, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 9: empty starts with empty",  str_starts_with(empty, empty) == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 10: empty ends with empty",   str_ends_with(empty, empty)   == 1, test_details, verbose);
    str_free(&empty);

    printf("\n    %s test: str_starts_with(), str_ends_with() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_str_contains(bool verbose) {
    printf("\n=== test: str_contains() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Basic contains
    append_formatted_text("basic contains checks\n\n", test_details, sizeof(test_details));
    String *s         = str("hello world");
    String *sub1      = str("hello");
    String *sub2      = str("world");
    String *sub3      = str("o w");
    String *sub4      = str("WORLD");
    String *sub_empty = str("");
    append_string_details("s",         s,         test_details, sizeof(test_details));
    append_string_details("sub1",      sub1,      test_details, sizeof(test_details));
    append_string_details("sub2",      sub2,      test_details, sizeof(test_details));
    append_string_details("sub3",      sub3,      test_details, sizeof(test_details));
    append_string_details("sub4",      sub4,      test_details, sizeof(test_details));
    append_string_details("sub_empty", sub_empty, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 1: contains prefix",        str_contains(s, sub1)      == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 2: contains suffix",        str_contains(s, sub2)      == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 3: contains middle",        str_contains(s, sub3)      == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 4: case mismatch",          str_contains(s, sub4)      == 0, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 5: contains empty string",  str_contains(s, sub_empty) == 1, test_details, verbose);
    str_free(&s, &sub1, &sub2, &sub3, &sub4, &sub_empty);

    // Empty string edge cases
    test_details[0] = '\0';
    append_formatted_text("empty string edge cases\n\n", test_details, sizeof(test_details));
    String *empty    = str("");
    String *nonempty = str("hello");
    String *empty2   = str("");
    append_string_details("empty",    empty,    test_details, sizeof(test_details));
    append_string_details("nonempty", nonempty, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 6: empty contains empty",        str_contains(empty, empty2)   == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 7: empty does not contain str",  str_contains(empty, nonempty) == 0, test_details, verbose);
    str_free(&empty, &nonempty, &empty2);

    // Complex pattern
    test_details[0] = '\0';
    append_formatted_text("complex pattern matching\n\n", test_details, sizeof(test_details));
    String *s2     = str("ababcabcabababd");
    String *sub2_1 = str("ababd");
    String *sub2_2 = str("abcab");
    String *sub2_3 = str("abcd");
    String *sub2_4 = str("");
    append_string_details("s2",     s2,     test_details, sizeof(test_details));
    append_string_details("sub2_1", sub2_1, test_details, sizeof(test_details));
    append_string_details("sub2_2", sub2_2, test_details, sizeof(test_details));
    append_string_details("sub2_3", sub2_3, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 8: complex pattern found",     str_contains(s2, sub2_1) == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 9: complex pattern found",     str_contains(s2, sub2_2) == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 10: complex pattern not found", str_contains(s2, sub2_3) == 0, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 11: complex contains empty",   str_contains(s2, sub2_4) == 1, test_details, verbose);
    str_free(&s2, &sub2_1, &sub2_2, &sub2_3, &sub2_4);

    // UTF-8 contains
    test_details[0] = '\0';
    append_formatted_text("utf-8 contains checks\n\n", test_details, sizeof(test_details));
    String *u   = str("東京。café 😀 大阪");
    String *uu1 = str("café");
    String *uu2 = str("😀");
    String *uu3 = str("京都");
    append_string_details("u",   u,   test_details, sizeof(test_details));
    append_string_details("uu1", uu1, test_details, sizeof(test_details));
    append_string_details("uu2", uu2, test_details, sizeof(test_details));
    append_string_details("uu3", uu3, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 12: utf-8 contains accented", str_contains(u, uu1) == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 13: utf-8 contains emoji",    str_contains(u, uu2) == 1, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 14: utf-8 not contained",     str_contains(u, uu3) == 0, test_details, verbose);
    str_free(&u, &uu1, &uu2, &uu3);

    printf("\n    %s test: str_contains() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_str_count(bool verbose) {
    printf("\n=== test: str_count() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Basic ASCII counts
    append_formatted_text("basic ascii counts\n\n", test_details, sizeof(test_details));
    String *s    = str("abababacaca");
    String *sub1 = str("ab");
    String *sub2 = str("a");
    String *sub3 = str("abc");
    String *sub4 = str("x");
    append_string_details("s",    s,    test_details, sizeof(test_details));
    append_string_details("sub1", sub1, test_details, sizeof(test_details));
    append_string_details("sub2", sub2, test_details, sizeof(test_details));
    append_string_details("sub3", sub3, test_details, sizeof(test_details));
    append_string_details("sub4", sub4, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 1: count \"ab\" = 3",  str_count(s, sub1) == 3, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 2: count \"a\" = 6",   str_count(s, sub2) == 6, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 3: count \"abc\" = 0", str_count(s, sub3) == 0, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 4: count \"x\" = 0",   str_count(s, sub4) == 0, test_details, verbose);
    str_free(&s, &sub1, &sub2, &sub3, &sub4);

    // Non-overlapping
    test_details[0] = '\0';
    append_formatted_text("non-overlapping count\n\n", test_details, sizeof(test_details));
    String *s2   = str("aaaa");
    String *sub5 = str("aa");
    append_string_details("s2",   s2,   test_details, sizeof(test_details));
    append_string_details("sub5", sub5, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 5: non-overlapping \"aa\" in \"aaaa\" = 2", str_count(s2, sub5) == 2, test_details, verbose);
    str_free(&s2, &sub5);

    // Full string match
    test_details[0] = '\0';
    append_formatted_text("full string match\n\n", test_details, sizeof(test_details));
    String *s3   = str("hello");
    String *sub6 = str("hello");
    append_string_details("s3",   s3,   test_details, sizeof(test_details));
    append_string_details("sub6", sub6, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 6: full string match = 1", str_count(s3, sub6) == 1, test_details, verbose);
    str_free(&s3, &sub6);

    // Substr longer than s
    test_details[0] = '\0';
    append_formatted_text("substr longer than string\n\n", test_details, sizeof(test_details));
    String *s4   = str("hi");
    String *sub7 = str("hello");
    append_string_details("s4",   s4,   test_details, sizeof(test_details));
    append_string_details("sub7", sub7, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 7: substr longer than s = 0", str_count(s4, sub7) == 0, test_details, verbose);
    str_free(&s4, &sub7);

    // UTF-8 multibyte
    test_details[0] = '\0';
    append_formatted_text("utf-8 multibyte counts\n\n", test_details, sizeof(test_details));
    String *s5   = str("café café café");
    String *sub8 = str("café");
    String *sub9 = str("é");
    append_string_details("s5",   s5,   test_details, sizeof(test_details));
    append_string_details("sub8", sub8, test_details, sizeof(test_details));
    append_string_details("sub9", sub9, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 8: count utf-8 word = 3", str_count(s5, sub8) == 3, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 9: count utf-8 rune = 3", str_count(s5, sub9) == 3, test_details, verbose);
    str_free(&s5, &sub8, &sub9);

    // Emoji counts
    test_details[0] = '\0';
    append_formatted_text("emoji counts\n\n", test_details, sizeof(test_details));
    String *s6    = str("😀🌍😀🌍😀");
    String *sub10 = str("😀");
    String *sub11 = str("🌍");
    append_string_details("s6",    s6,    test_details, sizeof(test_details));
    append_string_details("sub10", sub10, test_details, sizeof(test_details));
    append_string_details("sub11", sub11, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 10: count emoji 😀 = 3", str_count(s6, sub10) == 3, test_details, verbose);
    ASSERT_BOOL_TRUE("Test 11: count emoji 🌍 = 2", str_count(s6, sub11) == 2, test_details, verbose);
    str_free(&s6, &sub10, &sub11);

    // Empty string
    test_details[0] = '\0';
    append_formatted_text("empty string edge case\n\n", test_details, sizeof(test_details));
    String *s7    = str("");
    String *sub12 = str("a");
    append_string_details("s7",    s7,    test_details, sizeof(test_details));
    append_string_details("sub12", sub12, test_details, sizeof(test_details));
    ASSERT_BOOL_TRUE("Test 12: empty string count = 0", str_count(s7, sub12) == 0, test_details, verbose);
    str_free(&s7, &sub12);

    printf("\n    %s test: str_count() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_index_functions(bool verbose) {
    printf("\n=== test: str_index_of(), str_indices_of() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // first and last index_of
    append_formatted_text("str_index_of() first and last\n\n", test_details, sizeof(test_details));
    String *s    = str("abababacaca");
    String *sub1 = str("ab");
    String *sub2 = str("ac");
    String *sub3 = str("xyz");
    append_string_details("s",    s,    test_details, sizeof(test_details));
    append_string_details("sub1", sub1, test_details, sizeof(test_details));
    append_string_details("sub2", sub2, test_details, sizeof(test_details));
    append_string_details("sub3", sub3, test_details, sizeof(test_details));
    ASSERT_INT_EQ("Test 1: first \"ab\"",  str_index_of(s, sub1, "first"),  0, test_details, verbose);
    ASSERT_INT_EQ("Test 2: last \"ab\"",   str_index_of(s, sub1, "last"),   4, test_details, verbose);
    ASSERT_INT_EQ("Test 3: first \"ac\"",  str_index_of(s, sub2, "first"),  6, test_details, verbose);
    ASSERT_INT_EQ("Test 4: \"xyz\" not found", str_index_of(s, sub3, "first"), -1, test_details, verbose);

    // str_indices_of: all occurrences found
    test_details[0] = '\0';
    append_formatted_text("str_indices_of() all occurrences\n\n", test_details, sizeof(test_details));
    append_string_details("s",    s,    test_details, sizeof(test_details));
    append_string_details("sub1", sub1, test_details, sizeof(test_details));
    size_t count;
    size_t *indices = str_indices_of(s, sub1, &count);
    ASSERT_BOOL_TRUE("Test 5: indices not NULL",      indices != NULL,      test_details, verbose);
    ASSERT_INT_EQ(   "Test 6: indices count = 3",     count, 3,             test_details, verbose);
    if (indices && count == 3) {
        ASSERT_INT_EQ("Test 7: indices[0] = 0", indices[0], 0, test_details, verbose);
        ASSERT_INT_EQ("Test 8: indices[1] = 2", indices[1], 2, test_details, verbose);
        ASSERT_INT_EQ("Test 9: indices[2] = 4", indices[2], 4, test_details, verbose);
    }
    free(indices);

    // str_indices_of: not found -> NULL
    test_details[0] = '\0';
    append_formatted_text("str_indices_of() not found\n\n", test_details, sizeof(test_details));
    append_string_details("s",    s,    test_details, sizeof(test_details));
    append_string_details("sub3", sub3, test_details, sizeof(test_details));
    indices = str_indices_of(s, sub3, &count);
    ASSERT_BOOL_TRUE("Test 10: indices NULL for no match", indices == NULL, test_details, verbose);
    free(indices);

    str_free(&s, &sub1, &sub2, &sub3);

    printf("\n    %s test: str_index_of(), str_indices_of() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_split(bool verbose) {
    printf("\n=== test: str_split() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";
    size_t count;
    String **parts;

    // Basic ASCII split
    append_formatted_text("basic ASCII split\n\n", test_details, sizeof(test_details));
    String *s1     = str("one,two,three,four");
    String *delim1 = str(",");
    append_string_details("s1",     s1,     test_details, sizeof(test_details));
    append_string_details("delim1", delim1, test_details, sizeof(test_details));
    parts = str_split(s1, delim1, &count);
    ASSERT_INT_EQ("Test 1: basic split count = 4", count, 4, test_details, verbose);
    if (parts && count == 4) {
        ASSERT_STR_EQ("Test 1a: parts[0]", parts[0], "one",   test_details, verbose);
        ASSERT_STR_EQ("Test 1b: parts[1]", parts[1], "two",   test_details, verbose);
        ASSERT_STR_EQ("Test 1c: parts[2]", parts[2], "three", test_details, verbose);
        ASSERT_STR_EQ("Test 1d: parts[3]", parts[3], "four",  test_details, verbose);
    }
    for (size_t i = 0; i < count; i++) str_free(&parts[i]);
    free(parts);
    str_free(&s1, &delim1);

    // Multi-char delimiter
    test_details[0] = '\0';
    append_formatted_text("multi-char delimiter split\n\n", test_details, sizeof(test_details));
    String *s2     = str("one::two::three");
    String *delim2 = str("::");
    append_string_details("s2",     s2,     test_details, sizeof(test_details));
    append_string_details("delim2", delim2, test_details, sizeof(test_details));
    parts = str_split(s2, delim2, &count);
    ASSERT_INT_EQ("Test 2: multi-char delim count = 3", count, 3, test_details, verbose);
    if (parts && count == 3) {
        ASSERT_STR_EQ("Test 2a: parts[0]", parts[0], "one",   test_details, verbose);
        ASSERT_STR_EQ("Test 2b: parts[1]", parts[1], "two",   test_details, verbose);
        ASSERT_STR_EQ("Test 2c: parts[2]", parts[2], "three", test_details, verbose);
    }
    for (size_t i = 0; i < count; i++) str_free(&parts[i]);
    free(parts);
    str_free(&s2, &delim2);

    // Delimiter at start and end -> empty parts
    test_details[0] = '\0';
    append_formatted_text("delimiter at start and end\n\n", test_details, sizeof(test_details));
    String *s3     = str(",one,two,");
    String *delim3 = str(",");
    append_string_details("s3",     s3,     test_details, sizeof(test_details));
    append_string_details("delim3", delim3, test_details, sizeof(test_details));
    parts = str_split(s3, delim3, &count);
    ASSERT_INT_EQ("Test 3: delim at edges count = 4", count, 4, test_details, verbose);
    if (parts && count == 4) {
        ASSERT_STR_EQ("Test 3a: parts[0] empty", parts[0], "",    test_details, verbose);
        ASSERT_STR_EQ("Test 3b: parts[1]",       parts[1], "one", test_details, verbose);
        ASSERT_STR_EQ("Test 3c: parts[2]",       parts[2], "two", test_details, verbose);
        ASSERT_STR_EQ("Test 3d: parts[3] empty", parts[3], "",    test_details, verbose);
    }
    for (size_t i = 0; i < count; i++) str_free(&parts[i]);
    free(parts);
    str_free(&s3, &delim3);

    // No delimiter found -> single part
    test_details[0] = '\0';
    append_formatted_text("no delimiter found\n\n", test_details, sizeof(test_details));
    String *s4     = str("hello");
    String *delim4 = str(",");
    append_string_details("s4",     s4,     test_details, sizeof(test_details));
    append_string_details("delim4", delim4, test_details, sizeof(test_details));
    parts = str_split(s4, delim4, &count);
    ASSERT_INT_EQ("Test 4: no delim count = 1", count, 1, test_details, verbose);
    if (parts && count == 1) {
        ASSERT_STR_EQ("Test 4a: parts[0]", parts[0], "hello", test_details, verbose);
    }
    for (size_t i = 0; i < count; i++) str_free(&parts[i]);
    free(parts);
    str_free(&s4, &delim4);

    // Consecutive delimiters -> empty parts
    test_details[0] = '\0';
    append_formatted_text("consecutive delimiters\n\n", test_details, sizeof(test_details));
    String *s5     = str("one,,two");
    String *delim5 = str(",");
    append_string_details("s5",     s5,     test_details, sizeof(test_details));
    append_string_details("delim5", delim5, test_details, sizeof(test_details));
    parts = str_split(s5, delim5, &count);
    ASSERT_INT_EQ("Test 5: consecutive delims count = 3", count, 3, test_details, verbose);
    if (parts && count == 3) {
        ASSERT_STR_EQ("Test 5a: parts[0]",       parts[0], "one", test_details, verbose);
        ASSERT_STR_EQ("Test 5b: parts[1] empty", parts[1], "",    test_details, verbose);
        ASSERT_STR_EQ("Test 5c: parts[2]",       parts[2], "two", test_details, verbose);
    }
    for (size_t i = 0; i < count; i++) str_free(&parts[i]);
    free(parts);
    str_free(&s5, &delim5);

    // UTF-8 multibyte delimiter
    test_details[0] = '\0';
    append_formatted_text("utf-8 multibyte delimiter\n\n", test_details, sizeof(test_details));
    String *s6     = str("東京。大阪。京都");
    String *delim6 = str("。");
    append_string_details("s6",     s6,     test_details, sizeof(test_details));
    append_string_details("delim6", delim6, test_details, sizeof(test_details));
    parts = str_split(s6, delim6, &count);
    ASSERT_INT_EQ("Test 6: utf-8 delim count = 3", count, 3, test_details, verbose);
    if (parts && count == 3) {
        ASSERT_STR_EQ("Test 6a: parts[0]", parts[0], "東京", test_details, verbose);
        ASSERT_STR_EQ("Test 6b: parts[1]", parts[1], "大阪", test_details, verbose);
        ASSERT_STR_EQ("Test 6c: parts[2]", parts[2], "京都", test_details, verbose);
    }
    for (size_t i = 0; i < count; i++) str_free(&parts[i]);
    free(parts);
    str_free(&s6, &delim6);

    // UTF-8 content with ASCII delimiter
    test_details[0] = '\0';
    append_formatted_text("utf-8 content with ascii delimiter\n\n", test_details, sizeof(test_details));
    String *s7     = str("café,résumé,naïve");
    String *delim7 = str(",");
    append_string_details("s7",     s7,     test_details, sizeof(test_details));
    append_string_details("delim7", delim7, test_details, sizeof(test_details));
    parts = str_split(s7, delim7, &count);
    ASSERT_INT_EQ("Test 7: utf-8 content count = 3", count, 3, test_details, verbose);
    if (parts && count == 3) {
        ASSERT_STR_EQ("Test 7a: parts[0]", parts[0], "café",   test_details, verbose);
        ASSERT_STR_EQ("Test 7b: parts[1]", parts[1], "résumé", test_details, verbose);
        ASSERT_STR_EQ("Test 7c: parts[2]", parts[2], "naïve",  test_details, verbose);
    }
    for (size_t i = 0; i < count; i++) str_free(&parts[i]);
    free(parts);
    str_free(&s7, &delim7);

    // Emoji delimiter
    test_details[0] = '\0';
    append_formatted_text("emoji delimiter\n\n", test_details, sizeof(test_details));
    String *s8     = str("hello😀world😀!");
    String *delim8 = str("😀");
    append_string_details("s8",     s8,     test_details, sizeof(test_details));
    append_string_details("delim8", delim8, test_details, sizeof(test_details));
    parts = str_split(s8, delim8, &count);
    ASSERT_INT_EQ("Test 8: emoji delim count = 3", count, 3, test_details, verbose);
    if (parts && count == 3) {
        ASSERT_STR_EQ("Test 8a: parts[0]", parts[0], "hello", test_details, verbose);
        ASSERT_STR_EQ("Test 8b: parts[1]", parts[1], "world", test_details, verbose);
        ASSERT_STR_EQ("Test 8c: parts[2]", parts[2], "!",     test_details, verbose);
    }
    for (size_t i = 0; i < count; i++) str_free(&parts[i]);
    free(parts);
    str_free(&s8, &delim8);

    printf("\n    %s test: str_split() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_str_slice(bool verbose) {
    printf("\n=== test: str_slice() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Basic ASCII slices
    append_formatted_text("basic ASCII slices\n\n", test_details, sizeof(test_details));
    String *s1 = str("Hello, world!");
    append_string_details("s1", s1, test_details, sizeof(test_details));
    String *a = str_slice(s1, 0, 5);
    String *b = str_slice(s1, 7, 12);
    String *c = str_slice(s1, 0, 13);
    ASSERT_STR_EQ("Test 1: slice(0, 5)",  a, "Hello",         test_details, verbose);
    ASSERT_STR_EQ("Test 2: slice(7, 12)", b, "world",         test_details, verbose);
    ASSERT_STR_EQ("Test 3: slice(0, 13)", c, "Hello, world!", test_details, verbose);
    str_free(&s1, &a, &b, &c);

    // Empty slice (start == end)
    test_details[0] = '\0';
    append_formatted_text("empty slice (start == end)\n\n", test_details, sizeof(test_details));
    String *s2 = str("Hello");
    append_string_details("s2", s2, test_details, sizeof(test_details));
    String *d = str_slice(s2, 3, 3);
    ASSERT_STR_EQ("Test 4: slice(3, 3) empty", d, "", test_details, verbose);
    str_free(&s2, &d);

    // Out of bounds clamping
    test_details[0] = '\0';
    append_formatted_text("out of bounds clamping\n\n", test_details, sizeof(test_details));
    String *s3 = str("Hello");
    append_string_details("s3", s3, test_details, sizeof(test_details));
    String *e = str_slice(s3, 0, 100);
    String *f = str_slice(s3, 50, 60);
    ASSERT_STR_EQ("Test 5: slice(0, 100) clamps to full", e, "Hello", test_details, verbose);
    ASSERT_STR_EQ("Test 6: slice(50, 60) clamps to empty", f, "",     test_details, verbose);
    str_free(&s3, &e, &f);

    // end < start -> empty
    test_details[0] = '\0';
    append_formatted_text("end < start (should return empty)\n\n", test_details, sizeof(test_details));
    String *s4 = str("Hello");
    append_string_details("s4", s4, test_details, sizeof(test_details));
    String *g = str_slice(s4, 4, 2);
    ASSERT_STR_EQ("Test 7: slice(4, 2) empty", g, "", test_details, verbose);
    str_free(&s4, &g);

    // Single rune
    test_details[0] = '\0';
    append_formatted_text("single rune slice\n\n", test_details, sizeof(test_details));
    String *s5 = str("Hello");
    append_string_details("s5", s5, test_details, sizeof(test_details));
    String *h = str_slice(s5, 1, 2);
    ASSERT_STR_EQ("Test 8: slice(1, 2) single rune", h, "e", test_details, verbose);
    str_free(&s5, &h);

    // UTF-8 multibyte runes
    test_details[0] = '\0';
    append_formatted_text("utf-8 multibyte rune slices\n\n", test_details, sizeof(test_details));
    String *s6 = str("café");  // c=0 a=1 f=2 é=3
    append_string_details("s6", s6, test_details, sizeof(test_details));
    String *si = str_slice(s6, 0, 3);
    String *sj = str_slice(s6, 3, 4);
    String *sk = str_slice(s6, 1, 3);
    ASSERT_STR_EQ("Test 9:  slice(0, 3)",  si, "caf", test_details, verbose);
    ASSERT_STR_EQ("Test 10: slice(3, 4)",  sj, "é",   test_details, verbose);
    ASSERT_STR_EQ("Test 11: slice(1, 3)",  sk, "af",  test_details, verbose);
    str_free(&s6, &si, &sj, &sk);

    // UTF-8 emoji (4-byte runes)
    test_details[0] = '\0';
    append_formatted_text("utf-8 emoji slices\n\n", test_details, sizeof(test_details));
    String *s7 = str("hi😀bye");  // h=0 i=1 😀=2 b=3 y=4 e=5
    append_string_details("s7", s7, test_details, sizeof(test_details));
    String *sl = str_slice(s7, 2, 3);
    String *sm = str_slice(s7, 0, 2);
    String *sn = str_slice(s7, 3, 6);
    ASSERT_STR_EQ("Test 12: slice(2, 3) emoji", sl, "😀", test_details, verbose);
    ASSERT_STR_EQ("Test 13: slice(0, 2)",       sm, "hi",  test_details, verbose);
    ASSERT_STR_EQ("Test 14: slice(3, 6)",       sn, "bye", test_details, verbose);
    str_free(&s7, &sl, &sm, &sn);

    // Mixed CJK, emoji, accented
    test_details[0] = '\0';
    append_formatted_text("mixed utf-8 slices\n\n", test_details, sizeof(test_details));
    String *s8 = str("東京😀café");  // 東=0 京=1 😀=2 c=3 a=4 f=5 é=6
    append_string_details("s8", s8, test_details, sizeof(test_details));
    String *so = str_slice(s8, 0, 2);
    String *sp = str_slice(s8, 2, 3);
    String *sq = str_slice(s8, 3, 7);
    ASSERT_STR_EQ("Test 15: slice(0, 2) CJK",   so, "東京", test_details, verbose);
    ASSERT_STR_EQ("Test 16: slice(2, 3) emoji",  sp, "😀",  test_details, verbose);
    ASSERT_STR_EQ("Test 17: slice(3, 7) accented", sq, "café", test_details, verbose);
    str_free(&s8, &so, &sp, &sq);

    printf("\n    %s test: str_slice() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_fmt(bool verbose) {
    printf("\n=== test: fmt() ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // reusable buffers
    char buf1[128];
    char buf2[128];
    char buf3[128];
    char buf4[128];

    // Basic formatting
    append_formatted_text("basic formatting\n\n", test_details, sizeof(test_details));
    char *r1 = fmt(buf1, "hello %s", "world");
    ASSERT_TEXT_EQ("Test 1: basic format", r1, "hello world", test_details, verbose);
    char *r2 = fmt(buf2, "%d + %d = %d", 2, 3, 5);
    ASSERT_TEXT_EQ("Test 2: integer format", r2, "2 + 3 = 5", test_details, verbose);

    // Multiple fmt() calls in one line
    test_details[0] = '\0';
    append_formatted_text("multiple fmt() calls in one line\n\n", test_details, sizeof(test_details));
    char *multi_a = fmt(buf1, "A=%d", 10);
    char *multi_b = fmt(buf2, "B=%d", 20);
    ASSERT_TEXT_EQ("Test 3: multi-call A", multi_a, "A=10", test_details, verbose);
    ASSERT_TEXT_EQ("Test 4: multi-call B", multi_b, "B=20", test_details, verbose);

    // Nested fmt() calls (two-step)
    test_details[0] = '\0';
    append_formatted_text("nested fmt() calls\n\n", test_details, sizeof(test_details));
    char *nested_inner = fmt(buf3, "value=%d", 42);
    char *nested_outer = fmt(buf1, "wrapped(%s)", nested_inner);
    ASSERT_TEXT_EQ("Test 5: nested inner", nested_inner, "value=42", test_details, verbose);
    ASSERT_TEXT_EQ("Test 6: nested outer", nested_outer, "wrapped(value=42)", test_details, verbose);

    // Nested fmt() inline expression
    test_details[0] = '\0';
    append_formatted_text("inline nested fmt() expression\n\n", test_details, sizeof(test_details));
    char *nested_inline = fmt(buf1, "outer(%s)", fmt(buf2, "inner-%d", 7));
    ASSERT_TEXT_EQ("Test 7: inline nested", nested_inline, "outer(inner-7)", test_details, verbose);

    printf("\n    %s test: fmt() %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:",
        passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}

void test_memory_allocation_procedures(bool verbose) {
    printf("\n=== test: str() memory allocation procedures ===\n");
    int passed = 0, failed = 0;

    char test_details[1024] = "";

    // Test MEM_LINEAR
    append_formatted_text("Test MEM_LINEAR: grow and shrink to fit exactly\n\n", test_details, sizeof(test_details));
    String *linear = str("hello", .allocation_procedure = MEM_LINEAR);
    append_string_details("linear", linear, test_details, sizeof(test_details));

    // Append to trigger growth
    str_append(linear, " world!");
    append_string_details("linear after append", linear, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 1: MEM_LINEAR append", linear, "hello world!", test_details, verbose);

    // Remove to trigger shrink
    str_remove(linear, 5, 6); // Remove " world"
    append_string_details("linear after remove", linear, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 2: MEM_LINEAR remove", linear, "hello!", test_details, verbose);

    str_free(&linear);

    // Test MEM_TRAILING
    append_formatted_text("Test MEM_TRAILING: grow but never shrink\n\n", test_details, sizeof(test_details));
    String *trailing = str("hello", .allocation_procedure = MEM_TRAILING);
    append_string_details("trailing", trailing, test_details, sizeof(test_details));

    // Append to trigger growth
    str_append(trailing, " world!");
    append_string_details("trailing after append", trailing, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 3: MEM_TRAILING append", trailing, "hello world!", test_details, verbose);

    // Remove should not trigger shrink
    size_t cap_before_remove = trailing->cap;
    str_remove(trailing, 5, 6); // Remove " world"
    append_string_details("trailing after remove", trailing, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 4: MEM_TRAILING remove", trailing, "hello!", test_details, verbose);
    ASSERT_BOOL_TRUE("Test 5: MEM_TRAILING capacity unchanged after remove",
                     trailing->cap == cap_before_remove, test_details, verbose);

    str_free(&trailing);

    // Test MEM_DOUBLE
    append_formatted_text("Test MEM_DOUBLE: double capacity on growth, halve on shrink\n\n", test_details, sizeof(test_details));
    String *double_str = str("hello", .allocation_procedure = MEM_DOUBLE);
    append_string_details("double_str", double_str, test_details, sizeof(test_details));

    // Append to trigger growth
    size_t cap_before_append = double_str->cap;
    str_append(double_str, " world!");
    append_string_details("double_str after append", double_str, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 6: MEM_DOUBLE append", double_str, "hello world!", test_details, verbose);
    ASSERT_BOOL_TRUE("Test 7: MEM_DOUBLE capacity doubled after append",
                     double_str->cap >= cap_before_append * 2, test_details, verbose);

    // Remove to trigger shrink
    cap_before_remove = double_str->cap;
    str_remove(double_str, 5, 6); // Remove " world"
    append_string_details("double_str after remove", double_str, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 8: MEM_DOUBLE remove", double_str, "hello!", test_details, verbose);
    ASSERT_BOOL_TRUE("Test 9: MEM_DOUBLE capacity halved after remove",
                     double_str->cap <= cap_before_remove / 2, test_details, verbose);

    str_free(&double_str);

    // Test MEM_FIXED with cap (for 10 runes)
    append_formatted_text("Test MEM_FIXED with cap (for 10 runes)\n\n", test_details, sizeof(test_details));
    String *fixed_cap = str("hello", .allocation_procedure = MEM_FIXED, .cap = 10 * 4);  // 40 bytes for 10 runes
    append_string_details("fixed_cap", fixed_cap, test_details, sizeof(test_details));

    // Append within capacity (1 rune: space)
    str_append(fixed_cap, " ");
    append_string_details("fixed_cap after append (space)", fixed_cap, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 10: MEM_FIXED append within capacity", fixed_cap, "hello ", test_details, verbose);

    // Try to append beyond capacity (emoji is 1 rune, but total would be 7 runes + 2 = 9 <= 10)
    str_append(fixed_cap, "🌍");  // This should succeed (9 runes total)
    append_string_details("fixed_cap after append (emoji)", fixed_cap, test_details, sizeof(test_details));
    ASSERT_STR_EQ("Test 11: MEM_FIXED append within rune limit", fixed_cap, "hello 🌍", test_details, verbose);

    str_free(&fixed_cap);

    // Test MEM_FIXED with insufficient initial capacity
    append_formatted_text("Test MEM_FIXED with insufficient initial capacity\n\n", test_details, sizeof(test_details));
    String *fixed_small = str("this is a long string", .allocation_procedure = MEM_FIXED, .cap = 10);
    ASSERT_BOOL_TRUE("Test 12: MEM_FIXED with insufficient capacity returns NULL",
                     fixed_small == NULL, test_details, verbose);

    printf("\n    %s test: Memory Allocation Procedures %d passed, %d failed\n",
        failed > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:", passed, failed);
    all_passed_tests += passed;
    all_failed_tests += failed;
}



int main(void) {
    printf("====== string_util tests: ======\n");

    bool verbose = false; // set verbose to true for all tests (of a specific test) to print test details of a test even if it passes, by default test details are only printed if a test fails

    test_str_init(verbose);
    test_str_clone(verbose);
    test_append(verbose);
    test_prepend(verbose);
    test_concat(verbose);
    test_to_upper_to_lower(verbose);
    test_insert(verbose);
    test_replace(verbose);
    test_str_repeat(verbose);
    test_str_remove(verbose);
    test_str_trim(verbose);
    test_str_equals(verbose);
    test_is_empty(verbose);
    test_starts_and_ends_with(verbose);
    test_str_contains(verbose);
    test_str_count(verbose);
    test_index_functions(verbose);
    test_split(verbose);
    test_str_slice(verbose);
    test_fmt(verbose);
    test_memory_allocation_procedures(verbose);

    printf("\n====== All tests complete ======\n");
    printf("    %s %d passed, %d failed\n\n",
        all_failed_tests > 0 ? "❌ NOT ALL TESTS PASSED:" : "✅ ALL TESTS PASSED:",
        all_passed_tests, all_failed_tests);
    return 0;
}



