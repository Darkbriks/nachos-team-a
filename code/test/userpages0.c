#include "syscall.h"
#include "types.h"

#define NB_ITER 10

/* Simple test of automatic thread termination */
void thread_func(void *arg) {
    for (int i = 0; i < NB_ITER; i+= 1){
        PutChar('A' + (int) arg);
    }
}

int main() {

    posix_thread_t tid1, tid2;
    PthreadCreate(&tid1, NULL, (void *(*)(void *))thread_func, (void *)0);
    PthreadCreate(&tid2, NULL, (void *(*)(void *))thread_func, (void *)1);

    for (int i =0 ; i < NB_ITER; i+= 1){
        PutChar('C');
    }
    PthreadJoin(tid1, NULL);
    PthreadJoin(tid2, NULL);

    PutChar('D');
    PutChar('\n');
}
