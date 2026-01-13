#include "syscall.h"
#include "nos_stdlib.h"
#include "nos_stddef.h"
#include "pthread.h"

void func2(void *arg) {
    long long before_sleep;
    long long after_sleep;

    int thread_id = (int)(long)arg;

    printf_simple("Thread "); PutInt(thread_id); printf_simple(": Starting sleep test\n");

    GetCurrentTick(&before_sleep);
    Sleep(1000 * thread_id);
    GetCurrentTick(&after_sleep);

    printf_simple("Thread "); PutInt(thread_id); printf_simple(": Woke up after ");
    PutInt(1000 * thread_id);
    printf_simple(" ticks\n");

    printf_simple("Thread "); PutInt(thread_id); printf_simple(": Slept for ");
    PutInt(after_sleep - before_sleep);
    printf_simple(" ticks (expected ~"); PutInt(1000 * thread_id); printf_simple(")\n");

    pthread_exit(0);
}

void func1(void *arg) {
    for (int i = 1; i <= 5; i++) {
        int thread_id;
        pthread_create((pthread_t *)&thread_id, NULL, (void *(*)(void *))func2, (void *)(long)i);
        printf_simple("Main: Created thread with ID "); PutInt(thread_id); printf_simple("\n");
        Sleep(100);
    }
    pthread_exit(0);
}

int main() {
    printf_simple("=== Test Sleep syscall with multiple threads ===\n");

    int main_thread_id;
    pthread_create((pthread_t *)&main_thread_id, NULL, (void *(*)(void *))func1, NULL);
    pthread_join(main_thread_id, NULL);

    printf_simple("Main thread finished.\n");
    return 0;
}
