#include "syscall.h"
#include "nos_stddef.h"
#include "pthread.h"

int counter = 0;
int mutex_id;

void increment_thread(void *arg) {
    for (int i = 0; i < 1000; i++) {
        SemWait(mutex_id);
        counter++;
        SemPost(mutex_id);
    }
    pthread_exit(0);
}

int main() {
    mutex_id = SemInit(1);
    if (mutex_id < 0) {
        PutString("Erreur création mutex\n", 23);
        return -1;
    }

    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, (void *(*)(void *))increment_thread, NULL);
    pthread_create(&tid2, NULL, (void *(*)(void *))increment_thread, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    PutString("Counter final: ", 15);
    PutInt(counter);  // Devrait être 2000
    PutChar('\n');

    SemDestroy(mutex_id);

    return 0;
}
