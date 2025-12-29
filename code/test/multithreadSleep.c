#include "syscall.h"
#include "my_stdlib.h"

void func2(void *arg) {
    long long before_sleep;
    long long after_sleep;

    int thread_id = (int)(long)arg;

    my_printf("Thread "); PutInt(thread_id); my_printf(": Starting sleep test\n");

    GetCurrentTick(&before_sleep);
    Sleep(1000 * thread_id);
    GetCurrentTick(&after_sleep);

    my_printf("Thread "); PutInt(thread_id); my_printf(": Woke up after ");
    PutInt(1000 * thread_id);
    my_printf(" ticks\n");

    my_printf("Thread "); PutInt(thread_id); my_printf(": Slept for ");
    PutInt(after_sleep - before_sleep);
    my_printf(" ticks (expected ~"); PutInt(1000 * thread_id); my_printf(")\n");

    Pthread_exit(0);
}

void func1(void *arg) {
    for (int i = 1; i <= 5; i++) {
        int thread_id;
        Pthread_create((posix_thread_t *)&thread_id, nullptr, (void *(*)(void *))func2, (void *)(long)i);
        my_printf("Main: Created thread with ID "); PutInt(thread_id); my_printf("\n");
        Sleep(100);
    }
    Pthread_exit(0);
}

int main() {
    my_printf("=== Test Sleep syscall with multiple threads ===\n");

    int main_thread_id;
    Pthread_create((posix_thread_t *)&main_thread_id, nullptr, (void *(*)(void *))func1, nullptr);
    Pthread_join(main_thread_id, nullptr);

    my_printf("Main thread finished.\n");
    return 0;
}