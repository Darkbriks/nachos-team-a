#include "userSem.h"
#include "system.h"
#include "syscall.h"
#include "linked_list.h"
#include "synch.h"

// Global list to track valid semaphores for security
// Each process should have its own list, but for now we use a global one
static LinkedList<Semaphore> *validSemaphores = nullptr;
static Lock *semaphoreListLock = nullptr;

static void InitSemaphoreTracking() {
    if (validSemaphores == nullptr) {
        validSemaphores = new LinkedList<Semaphore>();
        semaphoreListLock = new Lock("semaphore list lock");
    }
}

static bool IsValidSemaphore(Semaphore *sem) {
    if (sem == nullptr) return false;

    semaphoreListLock->Acquire();
    Semaphore *found = validSemaphores->FindInList(sem);
    semaphoreListLock->Release();

    return (found != nullptr);
}

void handle_SC_SemInit(){
    ASSERT(sizeof(int) == 4 && 4 == sizeof(Semaphore *));

    InitSemaphoreTracking();

    int semUserAddr = machine->ReadRegister(4);
    int originalValue = machine->ReadRegister(5);

    if (semUserAddr < 0) {
        RETURN(-E_FAULT);
    }

    if (originalValue < 0) {
        RETURN(-E_INVAL);
    }

    Semaphore *newSem = new Semaphore("user_sem", originalValue);

    semaphoreListLock->Acquire();
    validSemaphores->AddInList(newSem);
    semaphoreListLock->Release();

    if (!machine->WriteMem(semUserAddr, sizeof(Semaphore *), (int) newSem)) {
        semaphoreListLock->Acquire();
        validSemaphores->RemoveInList(newSem);
        semaphoreListLock->Release();
        delete newSem;
        RETURN(-E_FAULT);
    }

    RETURN(0);
}

void handle_common(int curCase){
    ASSERT(sizeof(int) == 4 && 4 == sizeof(Semaphore *));
    int addr = machine->ReadRegister(4);
    if (addr < 0) {
        RETURN(-E_FAULT);
    }

    int x;
    if (!machine->ReadMem(addr, sizeof(Semaphore *), &x)) {
        RETURN(-E_FAULT);
    }

    Semaphore *sem = (Semaphore *) x;

    if (!IsValidSemaphore(sem)) {
        RETURN(-E_INVAL);
    }

    if (curCase == 0){
        sem->P();
    } else if (curCase == 1){
        sem->V();
    } else if (curCase == 2){
        semaphoreListLock->Acquire();
        validSemaphores->RemoveInList(sem);
        semaphoreListLock->Release();
        delete sem;
    }
    RETURN(0);

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
