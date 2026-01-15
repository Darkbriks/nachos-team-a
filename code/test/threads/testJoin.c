#include "syscall.h"
#include "nos_pthread.h"

void * thread_func(void *arg) {
    return (void *) 42;
}

int main() {
    pthread_t tid;
    void *retval;
    
    pthread_create(&tid, 0, thread_func, 0);
    PutString("hey\n", 5);
    pthread_join(tid, &retval);

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
