#include "syscall.h"
#include "nos_errno.h"
#include "test_utilities.h"

static void test_sem_init_valid_value(void) {
    TEST_START("sem_init_valid_value");

    CLEAR_ERRNO();
    int sem_id = SemInit(3);

    ASSERT_NON_NEGATIVE(sem_id, "SemInit with valid value should succeed");
    ASSERT_ERRNO(E_SUCCESS, "errno should be 0");

    TEST_PASS();
}

static void test_sem_init_negative_value(void) {
    TEST_START("sem_init_negative_value");

    CLEAR_ERRNO();
    int sem_id = SemInit(-1);

    ASSERT_NON_NEGATIVE(sem_id, "SemInit with negative value should succeed");
    ASSERT_ERRNO(E_SUCCESS, "errno should be 0");

    TEST_PASS();
}

static void test_sem_wait_valid_id(void) {
    TEST_START("sem_wait_valid_id");

    int sem_id = SemInit(1);
    ASSERT_NON_NEGATIVE(sem_id, "SemInit should succeed for SemWait test");

    CLEAR_ERRNO();
    int ret = SemWait(sem_id);

    ASSERT_NON_NEGATIVE(ret, "SemWait with valid semaphore ID should succeed");
    ASSERT_ERRNO(E_SUCCESS, "errno should be 0");

    SemDestroy(sem_id); // Cleanup
    TEST_PASS();
}

static void test_sem_wait_invalid_id(void) {
    TEST_START("sem_wait_invalid_id");

    CLEAR_ERRNO();
    int ret = SemWait(-999);

    ASSERT_ERROR(ret, "SemWait with invalid semaphore ID should fail");
    ASSERT_ERRNO(E_NOENT, "errno should be E_NOENT");

    TEST_PASS();
}

static void test_sem_post_valid_id(void) {
    TEST_START("sem_post_valid_id");

    int sem_id = SemInit(0);
    ASSERT_NON_NEGATIVE(sem_id, "SemInit should succeed for SemPost test");

    CLEAR_ERRNO();
    int ret = SemPost(sem_id);

    ASSERT_NON_NEGATIVE(ret, "SemPost with valid semaphore ID should succeed");
    ASSERT_ERRNO(E_SUCCESS, "errno should be 0");

    SemDestroy(sem_id); // Cleanup
    TEST_PASS();
}

static void test_sem_post_invalid_id(void) {
    TEST_START("sem_post_invalid_id");

    CLEAR_ERRNO();
    int ret = SemPost(-999);

    ASSERT_ERROR(ret, "SemPost with invalid semaphore ID should fail");
    ASSERT_ERRNO(E_NOENT, "errno should be E_NOENT");

    TEST_PASS();
}

static void test_sem_destroy_valid_id(void) {
    TEST_START("sem_destroy_valid_id");

    int sem_id = SemInit(1);
    ASSERT_NON_NEGATIVE(sem_id, "SemInit should succeed for SemDestroy test");

    CLEAR_ERRNO();
    int ret = SemDestroy(sem_id);

    ASSERT_NON_NEGATIVE(ret, "SemDestroy with valid semaphore ID should succeed");
    ASSERT_ERRNO(E_SUCCESS, "errno should be 0");

    TEST_PASS();
}

static void test_sem_destroy_invalid_id(void) {
    TEST_START("sem_destroy_invalid_id");

    CLEAR_ERRNO();
    int ret = SemDestroy(-999);

    ASSERT_ERROR(ret, "SemDestroy with invalid semaphore ID should fail");
    ASSERT_ERRNO(E_NOENT, "errno should be E_NOENT");

    TEST_PASS();
}

static void test_set_max_sem_for_process(void) {
    TEST_START("set_max_sem_for_process");

    CLEAR_ERRNO();
    int sem_id = SemInit(1);
    ASSERT_NON_NEGATIVE(sem_id, "SemInit should succeed for SetMaxSemForProcess test");

    CLEAR_ERRNO();
    int previous_max = SetMaxSemForProcess(32);
    ASSERT_NON_NEGATIVE(previous_max, "SetMaxSemForProcess should succeed");
    ASSERT_ERRNO(E_SUCCESS, "errno should be 0");

    CLEAR_ERRNO();
    int ret = SemPost(sem_id);
    ASSERT_EQ(ret, 0, "SemPost should still succeed after SetMaxSemForProcess");
    ASSERT_ERRNO(E_SUCCESS, "errno should be 0");

    TEST_PASS();
}

static void test_sem_wait_after_destroy(void) {
    TEST_START("sem_wait_after_destroy");

    int sem_id = SemInit(1);
    ASSERT_NON_NEGATIVE(sem_id, "SemInit should succeed for SemWait after destroy test");

    int ret = SemDestroy(sem_id);
    ASSERT_NON_NEGATIVE(ret, "SemDestroy should succeed before SemWait test");

    CLEAR_ERRNO();
    ret = SemWait(sem_id);

    ASSERT_ERROR(ret, "SemWait after SemDestroy should fail");
    ASSERT_ERRNO(E_NOENT, "errno should be E_NOENT");

    TEST_PASS();
}

static void test_sem_post_after_destroy(void) {
    TEST_START("sem_post_after_destroy");

    int sem_id = SemInit(1);
    ASSERT_NON_NEGATIVE(sem_id, "SemInit should succeed for SemPost after destroy test");

    int ret = SemDestroy(sem_id);
    ASSERT_NON_NEGATIVE(ret, "SemDestroy should succeed before SemPost test");

    CLEAR_ERRNO();
    ret = SemPost(sem_id);

    ASSERT_ERROR(ret, "SemPost after SemDestroy should fail");
    ASSERT_ERRNO(E_NOENT, "errno should be E_NOENT");

    TEST_PASS();
}

int main() {
    TEST_SUITE_START("Semaphore Syscall Tests");

    test_sem_init_valid_value();
    test_sem_init_negative_value();
    test_sem_wait_valid_id();
    test_sem_wait_invalid_id();
    test_sem_post_valid_id();
    test_sem_post_invalid_id();
    test_sem_destroy_valid_id();
    test_sem_destroy_invalid_id();
    test_set_max_sem_for_process();
    test_sem_wait_after_destroy();
    test_sem_post_after_destroy();

    TEST_SUITE_END();
}