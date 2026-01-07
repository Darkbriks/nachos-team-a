#include "syscall.h"
#include "types.h"

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
    PutString("=== Test pas de hiérarchie entre Threads ===\n", 35);
    pthread_attr_t attr;
    Pthread_attr_init(&attr);
    Pthread_attr_setdetachstate(&attr, DETACHED);
    if (PthreadCreate(NULL, &attr, simple_thread, (void *)1) != 0) {
        PutString("ERROR: Failed to create thread 1\n", 33);
        return 1;
    }
    PthreadExit(0);
}

