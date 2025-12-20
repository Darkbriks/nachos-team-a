#include "userSem.h"
#include "system.h"




void handle_SC_SemInit(){
    int sem = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
    Semaphore *newSem = new Semaphore("sem_thread", n);
    printf("ici %d, %d\n", sem, (int) newSem);
    machine->WriteMem(sem, sizeof(Semaphore *), (int) newSem);

}

void handle_SC_SemP(){
    int addr = machine->ReadRegister(4);
    Semaphore * sem  = (Semaphore *)  malloc(sizeof( Semaphore *));
    machine->ReadMem(addr , sizeof(Semaphore *), (int *) sem);
    printf("ici %d\n", addr);
    printf("ici %d\n", (int) sem);
    sem->P();
    printf("ici\n");
}

void handle_SC_SemV(){
    int addr = machine->ReadRegister(4);
    Semaphore * sem = (Semaphore *) addr;
    sem->V();
}

void handle_SC_SemDestroy(){
    int addr = machine->ReadRegister(4);
    Semaphore * sem = (Semaphore *) addr;
    delete sem;
}
