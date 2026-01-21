#include "syscall.h"
#include "nos_stddef.h"
#include "nos_pthread.h"
#include "test_utilities.h"

int counter = 0;
int mutex_id;

void increment_thread(void *arg) {
    for (int i = 0; i < 1000; i++) {
        SemWait(mutex_id);
        counter++;
        SemPost(mutex_id);
    }
    pthread_exit(0);
}

void check_final_counter() {
    TEST_START("check_final_counter");
    ASSERT_EQ(counter, 2000, "Final counter value should be 2000");
    TEST_PASS();
}

int main() {
    TEST_SUITE_START("Thread Semaphore Synchronization Test");

    mutex_id = SemInit(1);
    if (mutex_id < 0) {
        PutString("Erreur création mutex\n", 23);
        return -1;
    }

    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, (void *(*)(void *))increment_thread, NULL);
    pthread_create(&tid2, NULL, (void *(*)(void *))increment_thread, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    check_final_counter();

    SemDestroy(mutex_id);

    TEST_SUITE_END();
}
