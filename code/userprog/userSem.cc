#include "userSem.h"

#include "syscall.h"
#include "system.h"

void handle_SC_SemInit() {
    int initialValue = machine->ReadRegister(4);
    if (initialValue < 0) { RETURN(-E_INVAL); }
    int handle = currentThread->getAddrSpace()->SemaphoreCreate(initialValue);
    RETURN(handle == -1 ? -E_FTABLE : handle);
}

void handle_SC_SemP() {
    int handle = machine->ReadRegister(4);
    bool success = currentThread->getAddrSpace()->SemaphoreWait(handle);
    RETURN(success ? 0 : -E_NOENT);
}

void handle_SC_SemV() {
    int handle = machine->ReadRegister(4);
    bool success = currentThread->getAddrSpace()->SemaphorePost(handle);
    RETURN(success ? 0 : -E_NOENT);
}

void handle_SC_SemDestroy() {
    int handle = machine->ReadRegister(4);
    bool success = currentThread->getAddrSpace()->SemaphoreDestroy(handle);
    RETURN(success ? 0 : -E_NOENT);
}