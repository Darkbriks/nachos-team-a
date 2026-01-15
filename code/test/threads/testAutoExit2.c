#include "syscall.h"
#include "nos_stddef.h"
#include "nos_pthread.h"

/* Simple test of automatic thread termination */
void thread_func(void *arg) {
    PutChar('A' + (int)arg);
}

int main() {
    PutChar('S');

    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, (void *(*)(void *))thread_func, (void *)0);
    pthread_create(&tid2, NULL, (void *(*)(void *))thread_func, (void *)1);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    PutChar('D');
    PutChar('\n');
}
