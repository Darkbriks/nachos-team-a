#include "syscall.h"
#include "types.h"

/* Simple test of automatic thread termination */
void thread_func(void *arg) {
    PutChar('A' + (int)arg);
}

int main() {
    PutChar('S');

    tid_t tid1, tid2;
    PthreadCreate(&tid1, NULL, (void *(*)(void *))thread_func, (void *)0);
    PthreadCreate(&tid2, NULL, (void *(*)(void *))thread_func, (void *)1);

    PthreadJoin(tid1, NULL);
    PthreadJoin(tid2, NULL);

    PutChar('D');
    PutChar('\n');
}