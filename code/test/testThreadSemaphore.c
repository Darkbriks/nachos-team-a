#include "syscall.h"

int counter = 0;
int mutex_id;

void increment_thread(void *arg) {
    for (int i = 0; i < 1000; i++) {
        SemP(mutex_id);
        counter++;
        SemV(mutex_id);
    }
    ExitThread();
}

int main() {
    mutex_id = SemInit(1);
    if (mutex_id < 0) {
        PutString("Erreur création mutex\n", 23);
        Exit(-1);
    }

    int tid1 = CreateThread(increment_thread, 0);
    int tid2 = CreateThread(increment_thread, 0);

    JoinThread(tid1);
    JoinThread(tid2);

    PutString("Counter final: ", 15);
    PutInt(counter);  // Devrait être 2000
    PutChar('\n');

    SemDestroy(mutex_id);

    return 0;
}