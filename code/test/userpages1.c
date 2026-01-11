#include "syscall.h"
#include "types.h"

#define NB_ITER 10

/* Simple test of automatic thread termination */
void thread_func(void *arg) {
    for (int i = 0; i < NB_ITER; i+= 1){
        PutChar('L' + (int) arg);
    }
}

int main() {

    tid_t tids[NB_ITER];
    for (int i = 0; i < NB_ITER; i++){
        PthreadCreate(&tids[i], NULL, (void *(*)(void *))thread_func, (void *)i);
        PthreadJoin(tids[i], NULL);
    }

    PutChar('C');

    PutChar('\n');
}
