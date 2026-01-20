#include "syscall.h"
#include "nos_stddef.h"
#include "nos_pthread.h"
#include "nos_stdio.h"

void func2(void *arg) {
    long long before_sleep;
    long long after_sleep;

    int thread_id = (int)(long)arg;

    printf("Thread "); PutInt(thread_id); printf(": Starting sleep test\n");

    GetCurrentTick(&before_sleep);
    Sleep(1000 * thread_id);
    GetCurrentTick(&after_sleep);

    printf("Thread %d: Woke up after %d ticks\n", thread_id, 1000 * thread_id);
    printf("Thread %d: Slept for %d ticks (expected ~%d)\n", thread_id, (int)(after_sleep - before_sleep), 1000 * thread_id);

    pthread_exit(0);
}

void func1(void *arg) {
    for (int i = 1; i <= 5; i++) {
        int thread_id;
        pthread_create((pthread_t *)&thread_id, NULL, (void *(*)(void *))func2, (void *)(long)i);
        printf("Main: Created thread with ID "); PutInt(thread_id); printf("\n");
        Sleep(100);
    }
    pthread_exit(0);
}

int main() {
    printf("=== Test Sleep syscall with multiple threads ===\n");

    int main_thread_id;
    pthread_create((pthread_t *)&main_thread_id, NULL, (void *(*)(void *))func1, NULL);
    pthread_join(main_thread_id, NULL);

    printf("Main thread finished.\n");
    return 0;
}
