// exception.cc
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "syscall.h"
#include "system.h"

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------
static void UpdatePC() {
    int pc = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, pc);
    pc = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, pc);
    pc += 4;
    machine->WriteRegister(NextPCReg, pc);
}

//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions
//      are in machine.h.
//----------------------------------------------------------------------

void handler_SC_exit() {
    int return_code = machine->ReadRegister(4);
	machine->WriteRegister(2, return_code);
    interrupt->Halt();
}    

void handler_SC_putString(){
    int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
    int offset = 0;
    char buffer[MAX_STRING_SIZE];

    while (offset < n) {
        copyStringFromMachine(addr + offset, buffer, MAX_STRING_SIZE);
        DEBUG('a', "PutString got string: %s\n", buffer);
		if (int res = synchConsole->SynchPutString(buffer, MAX_STRING_SIZE); res <= 0) { n = -1; break; }
		else { offset += res; }
    }
	machine->WriteRegister(2, n);
}

void handler_SC_getString(){
	int addr = machine->ReadRegister(4);
    int n = machine->ReadRegister(5);
	if (n > MAX_STRING_SIZE) { n = MAX_STRING_SIZE; } // TODO Set errno when we have a library for user
    {
		char buffer[n];
        int res = synchConsole->SynchGetString(buffer, n);
        copyStringToMachine(buffer, addr, n);
		machine->WriteRegister(2, res);
    }
}

void handler_SC_PutInt(){
    int value = machine->ReadRegister(4);
    char value_str[12]; // An integer is never bigger than 12 character.
    snprintf(value_str, 12, "%d", value);
    synchConsole->SynchPutString(value_str, 12); // TODO handle future errno exception
}

void handler_SC_GetInt(){
    ptr_32 addr = (ptr_32) machine->ReadRegister(4);
    char value[12]; // An integer is never bigger than 12 character.
    synchConsole->SynchGetString(value, 12); // TODO handle future errno exception
    int * new_val = new int;
    if (sscanf(value, "%d", new_val) !=1){
        // handle error don't find integer
        // TODO wait for errno
        ASSERT(FALSE);
    }
    machine->WriteMem(addr, 4, *new_val);
    delete new_val;
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2);

    if (which != SyscallException){ 
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
    switch (type){
        case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;
        case SC_Exit:
            DEBUG('a', "Exit, initiated by user program.\n");
            handler_SC_exit();
            break;
        case SC_PutChar:
            DEBUG('a', "PutChar exception.cc\n");
            synchConsole->SynchPutChar(machine->ReadRegister(4));
            break;

        case SC_PutString:
            DEBUG('a', "PutString exception.cc\n");
            handler_SC_putString();
            break;
        case SC_GetChar:
            DEBUG('a', "GetChar exception.cc\n");
            {
                char ch = synchConsole->SynchGetChar();
                machine->WriteRegister(2, (int) ch);
            }
            break;
        case SC_GetString:
            DEBUG('a', "GetString exception.cc\n");
            handler_SC_getString();
            break;

        case SC_PutInt:
            DEBUG('a', "PutInt exception.cc\n");
            handler_SC_PutInt();
            break;

        case SC_GetInt:
            DEBUG('a', "GetInt exception.cc\n");
            handler_SC_GetInt();
            break;

        default:
            printf("Unknow syscall :%d\n", type);
            ASSERT(FALSE);
            break;

    }

    // LB: Do not forget to increment the pc before returning!
    UpdatePC();
    // End of addition
}
