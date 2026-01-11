#include "syscall.h"

void *simple_thread(void *arg) {
    PutString("Thread executing\n", 17);
    return (void *)42;
}

void *joining_thread(void *arg) {
    tid_t tid = (tid_t)arg;
    PutString("Attempting to join from child thread\n", 38);
    return (void *)(long)PthreadJoin(tid, 0);
}

int main() {
    PutString("=== Test PthreadJoin Error Cases ===\n", 39);
    
    PutString("Test 1: Join non-existent thread\n", 34);
    tid_t fake_tid = 9999;
    
    if (PthreadJoin(fake_tid, 0) == 0) {
        PutString("ERROR: Join succeeded on non-existent thread\n", 46);
        return 1;
    }
    
    PutString("Test 1: PASS - Cannot join non-existent thread\n", 48);
    
    PutString("Test 2: Self-join\n", 15);
    tid_t self_tid = 0; // Main thread TID is assumed to be 0
    if (PthreadJoin(self_tid, 0) == 0) {
        PutString("ERROR: Self-join succeeded (should fail)\n", 42);
        return 1;
    }
    PutString("Test 2: PASS - Cannot self-join\n", 33);
    
    PutString("Test 3: Double join\n", 20);
    tid_t tid1;
    void *retval;
    
    if (PthreadCreate(&tid1, 0, simple_thread, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (PthreadJoin(tid1, &retval) != 0) {
        PutString("ERROR: First join failed\n", 25);
        return 1;
    }
    
    if ((int)retval != 42) {
        PutString("ERROR: Wrong return value from first join\n", 43);
        return 1;
    }
    
    if (PthreadJoin(tid1, 0) == 0) {
        PutString("ERROR: Second join succeeded (should fail)\n", 44);
        return 1;
    }
    
    PutString("Test 3: PASS - Cannot join twice\n", 34);
    
    PutString("Test 4: Two threads joining same thread\n", 41);
    tid_t tid2, tid3, tid4;
    void *ret1, *ret2;
    
    if (PthreadCreate(&tid2, 0, simple_thread, 0) != 0) {
        PutString("ERROR: Failed to create target thread\n", 39);
        return 1;
    }
    
    if (PthreadCreate(&tid3, 0, joining_thread, (void *)(long)tid2) != 0) {
        PutString("ERROR: Failed to create first joiner\n", 38);
        return 1;
    }
    
    if (PthreadCreate(&tid4, 0, joining_thread, (void *)(long)tid2) != 0) {
        PutString("ERROR: Failed to create second joiner\n", 39);
        return 1;
    }
    
    PthreadJoin(tid3, &ret1);
    PthreadJoin(tid4, &ret2);
    
    int success_count = 0;
    if ((int)(long)ret1 == 0) success_count++;
    if ((int)(long)ret2 == 0) success_count++;
    
    if (success_count != 1) {
        PutString("ERROR: Expected exactly one successful join, got ", 50);
        PutInt(success_count);
        PutChar('\n');
        return 1;
    }
    
    PutString("Test 4: PASS - Only one joiner succeeded\n", 42);
    
    PutString("Test 5: Join detached thread\n", 30);
    tid_t tid5;
    
    if (PthreadCreate(&tid5, 0, simple_thread, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (PthreadDetach(tid5) != 0) {
        PutString("ERROR: Failed to detach thread\n", 31);
        return 1;
    }
    
    if (PthreadJoin(tid5, 0) == 0) {
        PutString("ERROR: Join succeeded on detached thread\n", 42);
        return 1;
    }
    
    PutString("Test 5: PASS - Cannot join detached thread\n", 44);
    
    PutString("SUCCESS: All join error tests passed\n", 38);
    return 0;
}