#include "syscall.h"

/* Test automatic thread termination - thread function returns without calling ExitThread */

void thread_func(void *arg) {
    int id = (int)arg;
    PutString("Thread ", 7);
    PutInt(id);
    PutString(" starting\n", 10);

    // Do some work
    for (int i = 0; i < 3; i++) {
    // When thread function returns, it will automatically call ExitThread
        PutString("  Thread ", 9);
        PutInt(id);
        PutString(" iteration ", 11);
        PutInt(i);
        PutChar('\n');
    }

    PutString("Thread ", 7);
    PutInt(id);
    PutString(" done - returning (should auto-exit)\n", 37);

    // NOTE: We do NOT call ExitThread() explicitly!
    // The return should automatically call ExitThread
}

int main() {
    PutString("=== Test Automatic Thread Termination ===\n", 43);

    // Create threads that will return without calling ExitThread
    int tid1 = CreateThread(thread_func, (void *)1);
    int tid2 = CreateThread(thread_func, (void *)2);

    PutString("Main: Created threads ", 22);
    PutInt(tid1);
    PutString(" and ", 5);
    PutInt(tid2);
    PutChar('\n');

    // Wait for threads
    PutString("Main: Waiting for thread ", 25);
    PutInt(tid1);
    PutString("...\n", 4);
    JoinThread(tid1);

    PutString("Main: Waiting for thread ", 25);
    PutInt(tid2);
    PutString("...\n", 4);
    JoinThread(tid2);

    PutString("Main: All threads finished successfully!\n", 41);
}
