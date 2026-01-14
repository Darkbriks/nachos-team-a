#include "syscall.h"
#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_string.h"
#include "test_utilities.h"

/* ============================================================
 * Test: malloc/free basic
 * ============================================================
 */
static void test_malloc_basic(void) {
    TEST_START("malloc_basic");

    void *ptr = malloc(100);
    ASSERT_NOT_NULL(ptr, "malloc returned NULL");
    memset(ptr, 0xAA, 100);
    free(ptr);

    TEST_PASS();
}

/* ============================================================
 * Test: calloc
 * ============================================================
 */
static void test_calloc(void) {
    TEST_START("calloc");

    int *arr = (int*)calloc(10, sizeof(int));
    ASSERT_NOT_NULL(arr, "calloc returned NULL");

    int all_zero = 1;
    for (int i = 0; i < 10; i++) {
        if (arr[i] != 0) { all_zero = 0; }
    }
    ASSERT_EQ(all_zero, 1, "calloc not zeroed");

    free(arr);

    TEST_PASS();
}

/* ============================================================
 * Test: realloc
 * ============================================================
 */
static void test_realloc(void) {
    TEST_START("realloc");

    char *ptr = (char*)malloc(10);
    ASSERT_NOT_NULL(ptr, "malloc returned NULL");

    strcpy(ptr, "hello");

    ptr = (char*)realloc(ptr, 20);
    ASSERT_NOT_NULL(ptr, "realloc returned NULL");
    ASSERT_STREQ(ptr, "hello", "data not preserved");

    free(ptr);

    ptr = (char*)realloc(NULL, 10);
    ASSERT_NOT_NULL(ptr, "realloc(NULL) failed");
    free(ptr);

    TEST_PASS();
}

/* ============================================================
 * Test: multiple allocations
 * ============================================================
 */
static void test_malloc_multiple(void) {
    TEST_START("malloc_multiple");

    void *ptrs[10];

    for (int i = 0; i < 10; i++) {
        ptrs[i] = malloc(50 + i * 10);
        if (ptrs[i] == NULL) {
            TEST_FAIL("malloc returned NULL");
            return;
        }
    }

    for (int i = 9; i >= 0; i--) { free(ptrs[i]); }

    TEST_PASS();
}

/* ============================================================
 * Test: itoa
 * ============================================================
 */
static void test_itoa(void) {
    TEST_START("itoa");

    char buf[32];

    itoa(12345, buf, 10);
    ASSERT_STREQ(buf, "12345", "itoa decimal wrong");

    itoa(-42, buf, 10);
    ASSERT_STREQ(buf, "-42", "itoa negative wrong");

    itoa(255, buf, 16);
    ASSERT_STREQ(buf, "ff", "itoa hex wrong");

    itoa(7, buf, 2);
    ASSERT_STREQ(buf, "111", "itoa binary wrong");

    TEST_PASS();
}

/* ============================================================
 * Test: strtol
 * ============================================================
 */
static void test_strtol(void) {
    TEST_START("strtol");

    char *end;

    long val = strtol("12345", &end, 10);
    ASSERT_EQ(val, 12345L, "strtol decimal wrong");
    ASSERT_EQ(*end, '\0', "end pointer wrong");

    val = strtol("-999", NULL, 10);
    ASSERT_EQ(val, -999L, "strtol negative wrong");

    val = strtol("0xFF", NULL, 0);
    ASSERT_EQ(val, 255L, "strtol hex auto wrong");

    val = strtol("0777", NULL, 0);
    ASSERT_EQ(val, 511L, "strtol octal auto wrong");

    TEST_PASS();
}

/* ============================================================
 * Test: abs/labs
 * ============================================================
 */
static void test_abs(void) {
    TEST_START("abs");

    ASSERT_EQ(abs(42), 42, "abs positive wrong");
    ASSERT_EQ(abs(-42), 42, "abs negative wrong");
    ASSERT_EQ(abs(0), 0, "abs zero wrong");

    ASSERT_EQ(labs(-123456L), 123456L, "labs wrong");

    TEST_PASS();
}

/* ============================================================
 * Test: rand/srand
 * ============================================================
 */
static void test_rand(void) {
    TEST_START("rand");

    srand(12345);
    int r1 = rand();
    int r2 = rand();

    ASSERT_EQ(r1 != r2, 1, "rand not random");

    srand(12345);
    int r3 = rand();
    ASSERT_EQ(r1, r3, "rand not reproducible");

    ASSERT_EQ(r1 >= 0 && r1 <= RAND_MAX, 1, "rand out of range");

    TEST_PASS();
}

/* ============================================================
 * Main test runner
 * ============================================================
 */
int main(void) {
    PutString("\n", 1);
    PutString("========================================\n", 50);
    PutString("  NachOS stdlib Test\n", 30);
    PutString("========================================\n", 50);

    PutString("\n--- Memory Tests ---\n", 30);
    test_malloc_basic();
    test_calloc();
    test_realloc();
    test_malloc_multiple();

    PutString("\n--- Conversion Tests ---\n", 30);
    test_itoa();
    test_strtol();
    test_abs();
    test_rand();

    PutString("\n========================================\n", 50);
    PutString("Test Results: ", 20);
    PutInt(tests_passed);
    PutString(" / ", 3);
    PutInt(tests_run);
    PutString(" tests passed\n", 20);
    PutString("========================================\n", 50);

    if (tests_passed == tests_run) {
        PutString("All tests PASSED!\n", 20);
    } else {
        PutString("FAILED: ", 10);
        PutInt(tests_run - tests_passed);
        PutString(" tests\n", 10);
    }

    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}