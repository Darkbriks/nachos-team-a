#include "syscall.h"

void *return_zero(void *arg) {
    return 0;
}

void *return_positive(void *arg) {
    return (void *)123456;
}

void *return_negative(void *arg) {
    return (void *)(-789);
}

void *explicit_exit(void *arg) {
    PthreadExit((void *)999);
    PutString("ERROR: Code after PthreadExit executed!\n", 42);
    return (void *)(-1);
}

void *exit_in_middle(void *arg) {
    int counter = 0;
    for (int i = 0; i < 5; i++) {
        counter++;
        if (i == 2) {
            PthreadExit((void *)(long)counter);
        }
    }
    return (void *)(long)counter;
}

int main() {
    PutString("=== Test PthreadExit Return Values ===\n", 41);
    
    tid_t tid;
    void *retval;
    
    PutString("Test 1: Return 0\n", 17);
    if (PthreadCreate(&tid, 0, return_zero, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (PthreadJoin(tid, &retval) != 0) {
        PutString("ERROR: Join failed\n", 19);
        return 1;
    }
    
    if (retval != 0) {
        PutString("ERROR: Expected 0, got ", 24);
        PutInt((int)retval);
        PutChar('\n');
        return 1;
    }
    
    PutString("Test 1: PASS\n", 13);
    
    PutString("Test 2: Return positive value\n", 30);
    if (PthreadCreate(&tid, 0, return_positive, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (PthreadJoin(tid, &retval) != 0) {
        PutString("ERROR: Join failed\n", 19);
        return 1;
    }
    
    if ((int)retval != 123456) {
        PutString("ERROR: Expected 123456, got ", 29);
        PutInt((int)retval);
        PutChar('\n');
        return 1;
    }
    
    PutString("Test 2: PASS\n", 13);
    
    PutString("Test 3: Return negative value\n", 30);
    if (PthreadCreate(&tid, 0, return_negative, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (PthreadJoin(tid, &retval) != 0) {
        PutString("ERROR: Join failed\n", 19);
        return 1;
    }
    
    if ((int)retval != -789) {
        PutString("ERROR: Expected -789, got ", 27);
        PutInt((int)retval);
        PutChar('\n');
        return 1;
    }
    
    PutString("Test 3: PASS\n", 13);
    
    PutString("Test 4: Explicit PthreadExit\n", 31);
    if (PthreadCreate(&tid, 0, explicit_exit, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (PthreadJoin(tid, &retval) != 0) {
        PutString("ERROR: Join failed\n", 19);
        return 1;
    }
    
    if ((int)retval != 999) {
        PutString("ERROR: Expected 999, got ", 26);
        PutInt((int)retval);
        PutChar('\n');
        return 1;
    }
    
    PutString("Test 4: PASS - PthreadExit does not return\n", 45);
    
    PutString("Test 5: PthreadExit in middle of function\n", 44);
    if (PthreadCreate(&tid, 0, exit_in_middle, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (PthreadJoin(tid, &retval) != 0) {
        PutString("ERROR: Join failed\n", 19);
        return 1;
    }
    
    if ((int)retval != 3) {
        PutString("ERROR: Expected 3, got ", 24);
        PutInt((int)retval);
        PutChar('\n');
        return 1;
    }
    
    PutString("Test 5: PASS\n", 13);
    
    PutString("Test 6: Join with NULL retval pointer\n", 39);
    if (PthreadCreate(&tid, 0, return_positive, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (PthreadJoin(tid, 0) != 0) {
        PutString("ERROR: Join with NULL retval failed\n", 37);
        return 1;
    }
    
    PutString("Test 6: PASS - Join with NULL retval works\n", 44);
    
    PutString("SUCCESS: All exit/return value tests passed\n", 45);
    return 0;
}