#include "syscall.h"

/* Simple test of automatic thread termination */
void thread_func(void *arg) {
    PutChar('A' + (int)arg);
}

int main() {
    PutChar('S');

    posix_thread_t tid1, tid2;
    Pthread_create(&tid1, nullptr, (void *(*)(void *))thread_func, (void *)0);
    Pthread_create(&tid2, nullptr, (void *(*)(void *))thread_func, (void *)1);

    Pthread_join(tid1, nullptr);
    Pthread_join(tid2, nullptr);

    PutChar('D');
    PutChar('\n');
}