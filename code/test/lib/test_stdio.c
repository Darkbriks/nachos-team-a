#include "syscall.h"
#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_string.h"
#include "test_utilities.h"

/* ============================================================
 * Test: putchar / putc
 * ============================================================
 */
static void test_putchar(void) {
    TEST_START("putchar");

    int ret = putchar('A');
    ASSERT_EQ(ret, 'A', "putchar return wrong");

    ret = putc('B', stdout);
    ASSERT_EQ(ret, 'B', "putc return wrong");

    putchar('\n');

    TEST_PASS();
}

/* ============================================================
 * Test: puts
 * ============================================================
 */
static void test_puts(void) {
    TEST_START("puts");

    int ret = puts("Hello from puts");
    ASSERT_EQ(ret >= 0, 1, "puts returned negative");

    TEST_PASS();
}

/* ============================================================
 * Test: fputs
 * ============================================================
 */
static void test_fputs(void) {
    TEST_START("fputs");

    int ret = fputs("Hello from fputs", stdout);
    ASSERT_EQ(ret >= 0, 1, "fputs returned negative");

    fputs("\n", stdout);

    TEST_PASS();
}

/* ============================================================
 * Test: fputc
 * ============================================================
 */
static void test_fputc(void) {
    TEST_START("fputc");

    int ret = fputc('X', stdout);
    ASSERT_EQ(ret, 'X', "fputc return wrong");

    fputc('Y', stdout);
    fputc('Z', stdout);
    fputc('\n', stdout);

    TEST_PASS();
}

/* ============================================================
 * Test: fflush
 * ============================================================
 */
static void test_fflush(void) {
    TEST_START("fflush");

    fputs("Buffered...", stdout);

    PutString(" (now flushing)", 20);

    int ret = fflush(stdout);
    ASSERT_EQ(ret, 0, "fflush returned non-zero");

    fputs("flushed!\n", stdout);

    TEST_PASS();
}

/* ============================================================
 * Test: stderr (unbuffered)
 * ============================================================
 */
static void test_stderr(void) {
    TEST_START("stderr_unbuffered");

    fputs("stderr:", stderr);
    fputc('!', stderr);
    fputc('\n', stderr);

    TEST_PASS();
}

/* ============================================================
 * Test: clearerr/feof/ferror
 * ============================================================
 */
static void test_error_funcs(void) {
    TEST_START("clearerr_feof_ferror");

    ASSERT_EQ(feof(stdin), 0, "feof should be 0");
    ASSERT_EQ(ferror(stdin), 0, "ferror should be 0");

    clearerr(stdin);

    TEST_PASS();
}

/* ============================================================
 * Test: ungetc (basic)
 * ============================================================
 */
static void test_ungetc(void) {
    TEST_START("ungetc");

    int ret = ungetc('Z', stdin);
    ASSERT_EQ(ret, 'Z', "ungetc return wrong");

    ret = ungetc('Y', stdin);
    ASSERT_EQ(ret, EOF, "second ungetc should fail");

    int c = fgetc(stdin);
    ASSERT_EQ(c, 'Z', "fgetc after ungetc wrong");

    TEST_PASS();
}

/* ============================================================
 * Main test runner
 * ============================================================
 */
int main(void) {
    PutString("\n", 1);
    PutString("========================================\n", 41);
    PutString("  NachOS stdio/stdlib Test Suite\n", 34);
    PutString("========================================\n", 41);

    PutString("\n--- Output Tests ---\n", 21);
    test_putchar();
    test_puts();
    test_fputs();
    test_fputc();
    test_fflush();
    test_stderr();

    PutString("\n--- Stream State Tests ---\n", 28);
    test_error_funcs();
    test_ungetc();

    /* Summary */
    PutString("\n========================================\n", 41);
    PutString("Test Results: ", 14);
    PutInt(tests_passed);
    PutString(" / ", 3);
    PutInt(tests_run);
    PutString(" tests passed\n", 14);
    PutString("========================================\n", 41);

    if (tests_passed == tests_run) {
        PutString("All tests PASSED!\n", 18);
    } else {
        PutString("FAILED: ", 8);
        PutInt(tests_run - tests_passed);
        PutString(" tests\n", 7);
    }

    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}