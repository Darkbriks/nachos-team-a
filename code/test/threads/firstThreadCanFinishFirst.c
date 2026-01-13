#include "syscall.h"
#include "types.h"
#include "pthread.h"

int counter = 0;

void *simple_thread(void *arg) {
    int id = (int)arg;
    counter++;
    SleepUntil(172644);
    PutString("Thread ", 7);
    PutInt(id);
    PutString(" executed\n", 10);
    return (void *)(id * 2);
}

int main() {
    PutString("=== Test pas de hiérarchie entre Threads ===\n", 35);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, DETACHED);
    if (pthread_create(NULL, &attr, simple_thread, (void *)1) != 0) {
        PutString("ERROR: Failed to create thread 1\n", 33);
        return 1;
    }
    PutString("=== MainFinish ===\n", 35);
    long long tick;
    GetCurrentTick(&tick);
    PutInt((int) tick);
    pthread_exit(0);
}

