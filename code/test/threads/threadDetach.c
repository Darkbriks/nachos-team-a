#include "syscall.h"
#include "pthread.h"

int detached_executed = 0;

void *detached_thread(void *arg) {
    PutString("Detached thread executing\n", 26);
    detached_executed = 1;
    return 0;
}

int main() {
    PutString("=== Test pthread_detach ===\n", 29);
    
    pthread_t tid;
    
    PutString("Test 1: Detach then try to join\n", 33);
    if (pthread_create(&tid, 0, detached_thread, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (pthread_detach(tid) != 0) {
        PutString("ERROR: Failed to detach thread\n", 31);
        return 1;
    }
    
    if (pthread_join(tid, 0) == 0) {
        PutString("ERROR: Join succeeded on detached thread (should fail)\n", 56);
        return 1;
    }
    
    PutString("Test 1: PASS - Cannot join detached thread\n", 44);
    
    PutString("Test 2: Double detach\n", 22);
    pthread_t tid2;
    
    if (pthread_create(&tid2, 0, detached_thread, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }
    
    if (pthread_detach(tid2) != 0) {
        PutString("ERROR: First detach failed\n", 27);
        return 1;
    }
    
    if (pthread_detach(tid2) == 0) {
        PutString("ERROR: Second detach succeeded (should fail)\n", 46);
        return 1;
    }
    
    PutString("Test 2: PASS - Cannot detach twice\n", 36);
    
    PutString("Test 3: Detach non-existent thread\n", 36);
    pthread_t fake_tid = 9999;
    
    if (pthread_detach(fake_tid) == 0) {
        PutString("ERROR: Detach succeeded on non-existent thread\n", 48);
        return 1;
    }
    
    PutString("Test 3: PASS - Cannot detach non-existent thread\n", 50);
    
    PutString("SUCCESS: All detach tests passed\n", 34);
    return 0;
}
