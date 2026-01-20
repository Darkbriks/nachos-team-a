#include "copyright.h"
#include "nos_errno.h"
#include "system.h"
#include "nos_limits.h"
#include "syscall.h"
#include "process.h"
#include <time.h>
#include "userIO.h"

void handle_SC_PutChar(){
    synchConsole->SynchPutChar(static_cast<char>(machine->ReadRegister(4)));
}

void handle_SC_PutString(){
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);

    if (n > MAX_PUT_STRING) {
        n = MAX_PUT_STRING;
    }

    if (n < 0) { RETURN(-E_INVAL); }
    if (n == 0) { RETURN(0); }
    if (n > INT_MAX - addr) { RETURN(-E_OVERFLOW); } // Prevent overflow

    Process *process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, E_FAULT);

    const AddrSpace *space = process->getSpace();
    VALIDATE_ARG(space != nullptr, E_FAULT);

    VALIDATE_ARG(space->IsValidUserRange(static_cast<unsigned int>(addr), static_cast<unsigned int>(n)), E_FAULT);

    int offset = 0;
    char buffer[MAX_STRING_SIZE];

    while (offset < n) {
        if (!CopyStringFromUser(addr + offset, buffer, MAX_STRING_SIZE)) {
            RETURN(-E_FAULT);
        }
        if (const int res = synchConsole->SynchPutString(buffer, MAX_STRING_SIZE); res <= 0) { break; }
        else { offset += res; }
    }
    RETURN(n);
}

void handle_SC_GetChar(){
    machine->WriteRegister(2, synchConsole->SynchGetChar());
}

void handle_SC_GetString(){
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);

    if (n < 0) { RETURN(-E_INVAL); }
    if (n == 0) { RETURN(0); }

    Process *process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, E_FAULT);

    const AddrSpace *space = process->getSpace();
    VALIDATE_ARG(space != nullptr, E_FAULT);

    VALIDATE_ARG(space->IsValidUserRange(static_cast<unsigned int>(addr), static_cast<unsigned int>(n)), E_FAULT);

    if (n > MAX_STRING_SIZE) { n = MAX_STRING_SIZE; }
    {
        char buffer[n];
        int res = synchConsole->SynchGetString(buffer, n);
        if (!CopyStringToUser(buffer, addr, n)) {
            RETURN(-E_FAULT);
        }
        RETURN(res);
    }
}

void handle_SC_PutInt(){
    int value = machine->ReadRegister(4);
    char value_str[12]; // An integer is never bigger than 12 character.
    snprintf(value_str, 12, "%d", value);
    RETURN(synchConsole->SynchPutString(value_str, 12));
}

void handle_SC_GetInt(){
    ptr_32 addr = (ptr_32) machine->ReadRegister(4);

    Process *process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, E_FAULT);

    const AddrSpace *space = process->getSpace();
    VALIDATE_ARG(space != nullptr, E_FAULT);

    VALIDATE_ARG(space->IsValidUserRange(static_cast<unsigned int>(addr), sizeof(int)), E_FAULT);

    char value[12]; // An integer is never bigger than 12 character.
    synchConsole->SynchGetString(value, 12);
    int new_val;
    if (sscanf(value, "%d", &new_val) !=1){
        RETURN(-E_INVAL);
    }
    machine->WriteMem(addr, 4, new_val);

    RETURN(0);
}


void handle_SC_time(){
    int addr = machine->ReadRegister(4);

    GET_PROCESS_ADDRSPACE();

    VALIDATE_ARG(space->IsValidUserRange(static_cast<unsigned int>(addr), static_cast<unsigned int>(sizeof(long long))), E_FAULT);

    time_t local = time(NULL);

    /* Cast to 64-bit to handle both 32-bit and 64-bit time_t */
    long long local64 = static_cast<long long>(local);
    const int high = static_cast<int>(local64 >> 32);
    const int low = static_cast<int>(local64 & 0xFFFFFFFF);

    if (!machine->WriteMem(addr, 4, low) || !machine->WriteMem(addr + 4, 4, high)) {
        RETURN(-E_FAULT);
    }
    RETURN(0);
}
