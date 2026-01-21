#include "syscall.h"
#include "nos_errno.h"
#include "test_utilities.h"
#include "nos_limits.h"

static void test_sleep_positive_time(void) {
    TEST_START("sleep_positive_time");

    CLEAR_ERRNO();
    int ret = Sleep(100);

    ASSERT_NON_NEGATIVE(ret, "sleep avec temps positif devrait reussir");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

static void test_sleep_zero_time(void) {
    TEST_START("sleep_zero_time");

    CLEAR_ERRNO();
    int ret = Sleep(0);

    ASSERT_NON_NEGATIVE(ret, "sleep avec temps zero devrait reussir");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

static void test_sleep_negative_time(void) {
    TEST_START("sleep_negative_time");

    CLEAR_ERRNO();
    int ret = Sleep(-10);

    ASSERT_ERROR(ret, "sleep avec temps negatif devrait echouer");
    ASSERT_ERRNO(E_INVAL, "errno devrait etre E_INVAL");

    TEST_PASS();
}

static void test_sleep_large_time(void) {
    TEST_START("sleep_large_time");

    CLEAR_ERRNO();
    int ret = Sleep(INT_MAX/32);

    ASSERT_NON_NEGATIVE(ret, "sleep avec grand temps devrait reussir");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

int main()
{
    TEST_SUITE_START("Sleep Syscall Tests");

    test_sleep_positive_time();
    test_sleep_zero_time();
    test_sleep_negative_time();
    test_sleep_large_time();

    TEST_SUITE_END();
}