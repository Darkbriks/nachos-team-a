#include "syscall.h"

int join_sem;
int thread_executed = 0;

void *thread_func(void *arg) {
    PutString("Thread executing and terminating\n", 33);
    thread_executed = 1;
    SemPost(join_sem);
    return 0;
}

int main() {
    PutString("=== Test Join on Terminated Thread ===\n", 40);

    posix_thread_t tid;

    join_sem = SemInit(0);

    if (PthreadCreate(&tid, 0, thread_func, 0) != 0) {
        PutString("ERROR: Failed to create thread\n", 31);
        return 1;
    }

    SemWait(join_sem);

    if (PthreadJoin(tid, 0) != 0) {
        PutString("ERROR: Join failed on terminated thread\n", 40);
        return 1;
    }

    if (!thread_executed) {
        PutString("ERROR: Thread did not execute properly\n", 39);
        return 1;
    }

    PutString("SUCCESS: Joined on terminated thread successfully\n", 50);
    return 0;
}