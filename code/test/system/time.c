#include "syscall.h"
#include "nos_errno.h"
#include "test_utilities.h"
#include "nos_limits.h"

static void test_getcurrenttick_valid_address(void) {
    TEST_START("getcurrenttick_valid_address");

    long long tick;
    CLEAR_ERRNO();
    int ret = GetCurrentTick(&tick);

    ASSERT_NON_NEGATIVE(ret, "GetCurrentTick with valid address should succeed");
    ASSERT_ERRNO(E_SUCCESS, "errno should be 0");

    TEST_PASS();
}

static void test_getcurrenttick_null_pointer(void) {
    TEST_START("getcurrenttick_null_pointer");

    CLEAR_ERRNO();
    int ret = GetCurrentTick((long long*)0);

    ASSERT_ERROR(ret, "GetCurrentTick with NULL pointer should fail");
    ASSERT_ERRNO(E_FAULT, "errno should be E_FAULT");

    TEST_PASS();
}

static void test_getcurrenttick_invalid_address(void) {
    TEST_START("getcurrenttick_invalid_address");

    CLEAR_ERRNO();
    int ret = GetCurrentTick((long long*)INT_MAX);

    ASSERT_ERROR(ret, "GetCurrentTick with invalid address should fail");
    ASSERT_ERRNO(E_FAULT, "errno should be E_FAULT");

    TEST_PASS();
}

static void test_time_valid_address(void) {
    TEST_START("time_valid_address");

    long long current_time;
    CLEAR_ERRNO();
    int ret = time(&current_time);

    ASSERT_NON_NEGATIVE(ret, "time with valid address should succeed");
    ASSERT_ERRNO(E_SUCCESS, "errno should be 0");

    TEST_PASS();
}

static void test_time_null_pointer(void) {
    TEST_START("time_null_pointer");

    CLEAR_ERRNO();
    int ret = time((long long*)0);

    ASSERT_ERROR(ret, "time with NULL pointer should fail");
    ASSERT_ERRNO(E_FAULT, "errno should be E_FAULT");

    TEST_PASS();
}

static void test_time_invalid_address(void) {
    TEST_START("time_invalid_address");

    CLEAR_ERRNO();
    int ret = time((long long*)INT_MAX);

    ASSERT_ERROR(ret, "time with invalid address should fail");
    ASSERT_ERRNO(E_FAULT, "errno should be E_FAULT");

    TEST_PASS();
}

int main() {
    TEST_SUITE_START("Time Syscall Tests");

    test_getcurrenttick_valid_address();
    test_getcurrenttick_null_pointer();
    test_getcurrenttick_invalid_address();

    test_time_valid_address();
    test_time_null_pointer();
    test_time_invalid_address();

    TEST_SUITE_END();
}