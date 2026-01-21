#include "syscall.h"
#include "nos_errno.h"
#include "test_utilities.h"

static void test_sleep_unitl_positive_time(void) {
    TEST_START("sleep_until_positive_time");

    long long current_tick;
    CLEAR_ERRNO();
    int ret = GetCurrentTick(&current_tick);
    ASSERT_NON_NEGATIVE(ret, "GetCurrentTick devrait reussir");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    long long wake_time = current_tick + 100;

    CLEAR_ERRNO();
    ret = SleepUntil(wake_time);

    ASSERT_NON_NEGATIVE(ret, "SleepUntil avec temps futur devrait reussir");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

static void test_sleep_unitl_past_time(void) {
    TEST_START("sleep_until_past_time");

    long long current_tick;
    CLEAR_ERRNO();
    int ret = GetCurrentTick(&current_tick);
    ASSERT_NON_NEGATIVE(ret, "GetCurrentTick devrait reussir");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    long long wake_time = current_tick - 100;

    CLEAR_ERRNO();
    ret = SleepUntil(wake_time);

    ASSERT_NON_NEGATIVE(ret, "SleepUntil avec temps passe devrait reussir");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

static void test_sleep_unitl_negative_time(void) {
    TEST_START("sleep_until_negative_time");

    CLEAR_ERRNO();
    int ret = SleepUntil(-10);

    ASSERT_ERROR(ret, "SleepUntil avec temps negatif devrait echouer");
    ASSERT_ERRNO(E_INVAL, "errno devrait etre E_INVAL");

    TEST_PASS();
}

int main()
{
    TEST_SUITE_START("SleepUntil Syscall Tests");

    test_sleep_unitl_positive_time();
    test_sleep_unitl_past_time();
    test_sleep_unitl_negative_time();

    TEST_SUITE_END();
}