#include "syscall.h"
#include "test_utilities.h"

int counter = 0;
int mutex_id;

static void extend_semaphore_limit_test() {
    TEST_START("extend_semaphore_limit_test");

    const int max_semaphores = 512;
    int sem_ids[max_semaphores];
    for (int i = 0; i < max_semaphores; i++) {
        CLEAR_ERRNO();
        sem_ids[i] = SemInit(1);
        ASSERT_NON_NEGATIVE(sem_ids[i], "SemInit should succeed within limit");
    }
    CLEAR_ERRNO();
    int sem_id = SemInit(1);
    ASSERT_ERROR(sem_id, "SemInit beyond limit should fail");
    ASSERT_ERRNO(E_FTABLE, "errno should be E_FTABLE when exceeding limit");

    TEST_PASS();
}

int main() {
    TEST_SUITE_START("Thread Semaphore Extended Limit Test");

    extend_semaphore_limit_test();

    TEST_SUITE_END();
}