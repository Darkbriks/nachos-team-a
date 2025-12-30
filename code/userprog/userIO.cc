#include "copyright.h"
#include "syscall.h"
#include "system.h"

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

    if (addr < 0) { RETURN(-E_FAULT); } // TODO Add more checks (addr is in valid user space, for example)
    if (n < 0) { RETURN(-E_INVAL); }
    if (n == 0) { RETURN(0); }
    if (n > INT32_MAX - addr) { RETURN(-E_OVERFLOW); } // Prevent overflow

    int offset = 0;
    char buffer[MAX_STRING_SIZE];

    while (offset < n) {
        copyStringFromMachine(addr + offset, buffer, MAX_STRING_SIZE);
        DEBUG('a', "PutString got string: %s\n", buffer);
        if (int res = synchConsole->SynchPutString(buffer, MAX_STRING_SIZE); res <= 0) { break; }
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

    if (addr < 0) { RETURN(-E_FAULT); } // TODO Add more checks (addr is in valid user space, for example)
    if (n < 0) { RETURN(-E_INVAL); }
    if (n == 0) { RETURN(0); }

    if (n > MAX_STRING_SIZE) { n = MAX_STRING_SIZE; }
    {
        char buffer[n];
        int res = synchConsole->SynchGetString(buffer, n);
        copyStringToMachine(buffer, addr, n);
        RETURN(res);
    }
}

void handle_SC_PutInt(){
    int value = machine->ReadRegister(4);
    char value_str[12]; // An integer is never bigger than 12 character.
    snprintf(value_str, 12, "%d", value);
    RETURN(synchConsole->SynchPutString(value_str, 12)); // TODO handle future errno exception
}

void handle_SC_GetInt(){
    ptr_32 addr = (ptr_32) machine->ReadRegister(4);
    if (addr < 0) { RETURN(-E_FAULT); }
    char value[12]; // An integer is never bigger than 12 character.
    synchConsole->SynchGetString(value, 12); // TODO handle future errno exception
    int new_val;
    if (sscanf(value, "%d", &new_val) !=1){
        RETURN(-E_INVAL);
    }
    machine->WriteMem(addr, 4, new_val);

    RETURN(0);
}
