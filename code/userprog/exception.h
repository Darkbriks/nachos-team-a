#ifndef EXCEPTION_H
#define EXCEPTION_H
#include "system.h"

#define RETURN(value) \
        machine->WriteRegister(2, (int)(value)); \
        return;

#define VALIDATE_ARG(cond, errcode) \
    do { \
        if (!(cond)) { \
            DEBUG('e', "Exception handler: argument validation failed: " #cond "\n"); \
            RETURN(-(errcode)) \
        } \
    } while (0)

#define GET_PROCESS_ADDRSPACE()                        \
    Process *process = currentThread->getProcess();    \
    VALIDATE_ARG(process != nullptr, E_FAULT);         \
    const AddrSpace *space = process->getSpace();      \
    VALIDATE_ARG(space != nullptr, E_FAULT);

#define GET_PROCESS_ADDRSPACE_STACKMANAGER()           \
    Process *process = currentThread->getProcess();    \
    VALIDATE_ARG(process != nullptr, E_FAULT);         \
    const AddrSpace *space = process->getSpace();      \
    VALIDATE_ARG(space != nullptr, E_FAULT);           \
    StackManager *stackMgr = space->GetStackManager(); \
    VALIDATE_ARG(stackMgr != nullptr, E_FAULT);

#define MAX_PUT_STRING 8192

#endif // EXCEPTION_H
