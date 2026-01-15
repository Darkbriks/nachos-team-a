#include "syscall.h"
#include "nos_pthread.h"

void *simple_thread(void *arg) {
    PutString("Thread executing\n", 17);
    return (void *)42;
}

void *joining_thread(void *arg) {
    pthread_t tid = (tid_t)arg;
    PutString("Attempting to join from child thread\n", 38);
    return (void *)(long)pthread_join(tid, 0);
}

int main() {
    PutString("=== Test pthread_join Error Cases ===\n", 39);
    
    PutString("Test 1: Join non-existent thread\n", 34);
    pthread_t fake_tid = 9999;
    
    if (pthread_join(fake_tid, 0) == 0) {
        PutString("ERROR: Join succeeded on non-existent thread\n", 46);
        return 1;
    }
    
    PutString("Test 1: PASS - Cannot join non-existent thread\n", 48);
    
    PutString("Test 2: Self-join\n", 15);
    pthread_t self_tid = 0; // Main thread TID is assumed to be 0
    if (pthread_join(self_tid, 0) == 0) {
        PutString("ERROR: Self-join succeeded (should fail)\n", 42);
        return 1;
    }
    PutString("Test 2: PASS - Cannot self-join\n", 33);
    
    PutString("Test 3: Double join\n", 20);
    pthread_t tid1;
    void *retval;
    
    if (pthread_create(&tid1, 0, simple_thread, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (pthread_join(tid1, &retval) != 0) {
        PutString("ERROR: First join failed\n", 25);
        return 1;
    }
    
    if ((int)retval != 42) {
        PutString("ERROR: Wrong return value from first join\n", 43);
        return 1;
    }
    
    if (pthread_join(tid1, 0) == 0) {
        PutString("ERROR: Second join succeeded (should fail)\n", 44);
        return 1;
    }
    
    PutString("Test 3: PASS - Cannot join twice\n", 34);
    
    PutString("Test 4: Two threads joining same thread\n", 41);
    pthread_t tid2, tid3, tid4;
    void *ret1, *ret2;
    
    if (pthread_create(&tid2, 0, simple_thread, 0) != 0) {
        PutString("ERROR: Failed to create target thread\n", 39);
        return 1;
    }
    
    if (pthread_create(&tid3, 0, joining_thread, (void *)(long)tid2) != 0) {
        PutString("ERROR: Failed to create first joiner\n", 38);
        return 1;
    }
    
    if (pthread_create(&tid4, 0, joining_thread, (void *)(long)tid2) != 0) {
        PutString("ERROR: Failed to create second joiner\n", 39);
        return 1;
    }
    
    pthread_join(tid3, &ret1);
    pthread_join(tid4, &ret2);
    
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
    pthread_t tid5;
    
    if (pthread_create(&tid5, 0, simple_thread, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (pthread_detach(tid5) != 0) {
        PutString("ERROR: Failed to detach thread\n", 31);
        return 1;
    }
    
    if (pthread_join(tid5, 0) == 0) {
        PutString("ERROR: Join succeeded on detached thread\n", 42);
        return 1;
    }
    
    PutString("Test 5: PASS - Cannot join detached thread\n", 44);
    
    PutString("SUCCESS: All join error tests passed\n", 38);
    return 0;
}
