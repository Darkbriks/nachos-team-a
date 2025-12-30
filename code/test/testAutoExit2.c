#include "syscall.h"

/* Simple test of automatic thread termination */
void thread_func(void *arg) {
    PutChar('A' + (int)arg);
}

int main() {
    PutChar('S');

    posix_thread_t tid1, tid2;
    PthreadCreate(&tid1, nullptr, (void *(*)(void *))thread_func, (void *)0);
    PthreadCreate(&tid2, nullptr, (void *(*)(void *))thread_func, (void *)1);

    PthreadJoin(tid1, nullptr);
    PthreadJoin(tid2, nullptr);

    PutChar('D');
    PutChar('\n');
}