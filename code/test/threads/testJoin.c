#include "syscall.h"

void *thread_func(void *arg) {
    return (void *)42;
}

int main() {
    tid_t tid;
    void *retval;
    
    PthreadCreate(&tid, 0, thread_func, 0);
    PthreadJoin(tid, &retval);

    if (retval != (void *)42) {
        PutString("Test failed: incorrect return value from joined thread: ", 100);
        PutInt((int)retval);
        PutChar('\n');
        return 1;
    } else {
        PutString("Test passed: correct return value from joined thread: ", 100);
        PutInt((int)retval);
        PutChar('\n');
        return 0;
    }
}