#include "syscall.h"

/* Simple test of automatic thread termination */
/*
void thread_func(void *arg) {
    PutChar('A' + (int)arg);
}

int main() {
    PutChar('S');

    int tid1 = CreateThread(thread_func, (void *)0);
    int tid2 = CreateThread(thread_func, (void *)1);

    JoinThread(tid1);
    JoinThread(tid2);

    PutChar('D');
    PutChar('\n');
}
*/

int main() {}