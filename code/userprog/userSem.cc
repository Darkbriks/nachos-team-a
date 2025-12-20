#include "userSem.h"
#include "system.h"




void handle_SC_SemInit(){
    ASSERT(sizeof(int) == 4 && 4 == sizeof(Semaphore *));
    int semUserAddr = machine->ReadRegister(4);
    int originalValue = machine->ReadRegister(5);
    Semaphore *newSem = new Semaphore("sem_thread", originalValue);
    printf("ici %d, %d\n", semUserAddr, (int) newSem);
    machine->WriteMem(semUserAddr, sizeof(Semaphore *), (int) newSem);
}

void handle_common(int curCase){
    ASSERT(sizeof(int) == 4 && 4 == sizeof(Semaphore *));
    int addr = machine->ReadRegister(4);
    int x;

    machine->ReadMem(addr , sizeof(Semaphore *), &x);
    Semaphore * sem = (Semaphore *) x;

    if (curCase == 0){
        sem->P();
    } else if (curCase == 1){
        sem->V();
    } else if (curCase == 2){
        delete sem;
    }
}


void handle_SC_SemP(){
    handle_common(0);
}

void handle_SC_SemV(){
    handle_common(1);
}

void handle_SC_SemDestroy(){
    handle_common(2);
}
