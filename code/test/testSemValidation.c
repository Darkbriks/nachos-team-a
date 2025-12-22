#include "syscall.h"

// Test 1: Normal semaphore usage
void test_normal_usage() {
    sem_t sem;
    PutString("Test 1: Normal semaphore usage\n", 35);

    int ret = SemInit(&sem, 1);
    if (ret != 0) {
        PutString("FAIL: SemInit failed\n", 22);
        return;
    }

    ret = SemP(&sem);
    if (ret != 0) {
        PutString("FAIL: SemP failed\n", 19);
        return;
    }

    ret = SemV(&sem);
    if (ret != 0) {
        PutString("FAIL: SemV failed\n", 19);
        return;
    }
    ret = SemDestroy(&sem);
    if (ret != 0) {
        PutString("FAIL: SemDestroy failed\n", 25);
        return;
    }

    PutString("PASS: Normal usage works\n", 27);
}

// Test 2: Invalid initial value
void test_invalid_init() {
    sem_t sem;
    PutString("Test 2: Invalid initial value\n", 32);

    int ret = SemInit(&sem, -5);
    if (ret == 0) {
        PutString("FAIL: SemInit accepted negative value\n", 40);
        SemDestroy(&sem);
        return;
    }

    PutString("PASS: Rejected negative initial value\n", 40);
}

// Test 3: Using destroyed semaphore (should fail)
void test_use_after_destroy() {
    sem_t sem;
    PutString("Test 3: Use after destroy\n", 28);
    SemInit(&sem, 1);
    SemDestroy(&sem);
    int ret = SemP(&sem);
    if (ret == 0) {
        PutString("FAIL: Used destroyed semaphore\n", 33);
        return;
    }
    PutString("PASS: Rejected destroyed semaphore\n", 37);
}

// Test 4: Invalid pointer (NULL-like)
void test_invalid_pointer() {
    PutString("Test 4: Invalid semaphore pointer\n", 36);
    sem_t sem = 0xDEADBEEF;
    int ret = SemP(&sem);
    if (ret == 0) {
        PutString("FAIL: Accepted invalid pointer\n", 33);
        return;
    }
    PutString("PASS: Rejected invalid pointer\n", 33);
}

// Test 5: Multi-threaded synchronization
void thread_func(void *arg) {
    sem_t *sem = (sem_t *)arg;
    PutString("  Thread: Waiting on semaphore...\n", 36);
    SemP(sem);
    PutString("  Thread: Got semaphore\n", 27);
    SemV(sem);
    PutString("  Thread: Released semaphore\n", 31);
}

void test_threading() {
    sem_t sem;
    PutString("Test 5: Multi-threaded usage\n", 31);

    SemInit(&sem, 0);
    PutString("  Main: Creating thread...\n", 29);
    CreateThread(thread_func, &sem);

    PutString("  Main: Sleeping briefly...\n", 30);
    Sleep(100);

    PutString("  Main: Signaling semaphore\n", 30);
    SemV(&sem);

    Sleep(200); // Give thread time to finish

    SemDestroy(&sem);
    PutString("PASS: Multi-threaded test complete\n", 37);
}

int main() {
    PutString("\n=== Semaphore Validation Tests ===\n\n", 39);

    test_normal_usage();
    PutString("\n", 1);

    test_invalid_init();
    PutString("\n", 1);

    test_use_after_destroy();
    PutString("\n", 1);

    test_invalid_pointer();
    PutString("\n", 1);

    test_threading();
    PutString("\n", 1);

    PutString("=== All tests completed ===\n", 30);

    return 0;
}
