#include "syscall.h"

int counter = 0;
int mutex_id;

void increment_thread(void *arg) {
    for (int i = 0; i < 1000; i++) {
        SemP(mutex_id);
        counter++;
        SemV(mutex_id);
    }
    Pthread_exit(0);
}

int main() {
    mutex_id = SemInit(1);
    if (mutex_id < 0) {
        PutString("Erreur création mutex\n", 23);
        return -1;
    }

    posix_thread_t tid1, tid2;
    Pthread_create(&tid1, nullptr, (void *(*)(void *))increment_thread, nullptr);
    Pthread_create(&tid2, nullptr, (void *(*)(void *))increment_thread, nullptr);

    Pthread_join(tid1, nullptr);
    Pthread_join(tid2, nullptr);

    PutString("Counter final: ", 15);
    PutInt(counter);  // Devrait être 2000
    PutChar('\n');

    SemDestroy(mutex_id);

    return 0;
}