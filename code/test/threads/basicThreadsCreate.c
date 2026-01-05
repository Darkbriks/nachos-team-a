#include "syscall.h"

int counter = 0;

void *simple_thread(void *arg) {
    int id = (int)arg;
    counter++;
    PutString("Thread ", 7);
    PutInt(id);
    PutString(" executed\n", 10);
    return (void *)(id * 2);
}

int main() {
    PutString("=== Test PthreadCreate Basic ===\n", 35);
    
    posix_thread_t tid1, tid2, tid3;
    void *retval1, *retval2, *retval3;
    
    if (PthreadCreate(&tid1, 0, simple_thread, (void *)1) != 0) {
        PutString("ERROR: Failed to create thread 1\n", 33);
        return 1;
    }
    
    if (PthreadCreate(&tid2, 0, simple_thread, (void *)2) != 0) {
        PutString("ERROR: Failed to create thread 2\n", 33);
        return 1;
    }
    
    if (PthreadCreate(&tid3, 0, simple_thread, (void *)3) != 0) {
        PutString("ERROR: Failed to create thread 3\n", 33);
        return 1;
    }
    
    PthreadJoin(tid1, &retval1);
    PthreadJoin(tid2, &retval2);
    PthreadJoin(tid3, &retval3);
    
    if ((int)retval1 != 2 || (int)retval2 != 4 || (int)retval3 != 6) {
        PutString("ERROR: Invalid return values\n", 29);
        return 1;
    }
    
    if (counter != 3) {
        PutString("ERROR: Not all threads executed (counter=", 42);
        PutInt(counter);
        PutString(")\n", 2);
        return 1;
    }
    
    PutString("SUCCESS: All threads created and joined correctly\n", 51);
    return 0;
}