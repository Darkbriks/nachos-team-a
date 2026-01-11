#include "syscall.h"
#include "types.h"

int counter = 0;
int mutex_id;

void increment_thread(void *arg) {
    for (int i = 0; i < 1000; i++) {
        SemWait(mutex_id);
        counter++;
        SemPost(mutex_id);
    }
    PthreadExit(0);
}

int main() {
    mutex_id = SemInit(1);
    if (mutex_id < 0) {
        PutString("Erreur création mutex\n", 23);
        return -1;
    }

    tid_t tid1, tid2;
    PthreadCreate(&tid1, NULL, (void *(*)(void *))increment_thread, NULL);
    PthreadCreate(&tid2, NULL, (void *(*)(void *))increment_thread, NULL);

    PthreadJoin(tid1, NULL);
    PthreadJoin(tid2, NULL);

    PutString("Counter final: ", 15);
    PutInt(counter);  // Devrait être 2000
    PutChar('\n');

    SemDestroy(mutex_id);

    return 0;
}