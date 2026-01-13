#include "thread.h"
#include "userSbrk.h"
#include "system.h"
#include "addrspace.h"
#include "exception.h"
#include "syscall.h"
#include "process.h"
#include "stackmanager.h"

void handle_SC_Sbrk() {
    int n = machine->ReadRegister(4);

    AddrSpace* space = currentThread->getAddrSpace();
    if (space == nullptr) { RETURN(-E_INVAL); }
    if (n < 0) { RETURN(-E_INVAL); }

    int result = space->Sbrk(n);
    if (result == -1) { RETURN(-E_NOMEM); }

    RETURN(result);
}

void handle_SC_mmap() {
    void* addr = reinterpret_cast<void*>(machine->ReadRegister(4));
    int length = machine->ReadRegister(5);

    Process* process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, -E_FAULT);

    AddrSpace* space = process->getSpace();
    VALIDATE_ARG(space != nullptr, -E_FAULT);

    StackManager* stackMgr = space->GetStackManager();
    VALIDATE_ARG(stackMgr != nullptr, -E_FAULT);

    // TODO: For now, we just allocate an area in the stack, without check addr
    //       In the future, we could implement a more complete mmap handling
    unsigned int limit;
    int result = stackMgr->AllocateStack(length, reinterpret_cast<unsigned int*>(&addr), &limit);
    if (result < 0) { RETURN(-E_NOMEM); }
    RETURN(reinterpret_cast<int>(addr));
}

void handle_SC_munmap() {
    unsigned int addr = static_cast<unsigned int>(machine->ReadRegister(4));

    Process* process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, -E_FAULT);

    AddrSpace* space = process->getSpace();
    VALIDATE_ARG(space != nullptr, -E_FAULT);

    StackManager* stackMgr = space->GetStackManager();
    VALIDATE_ARG(stackMgr != nullptr, -E_FAULT);

    int result = stackMgr->FreeStack(addr);
    if (result < 0) { RETURN(-E_INVAL); }
    RETURN(0);
}