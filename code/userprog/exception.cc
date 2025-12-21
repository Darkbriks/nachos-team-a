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
#include "userthread.h"
#include "process.h"
#include "exception.h"
#include "userIO.h"
#include "userSleep.h"
#include "userSem.h"

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
    const int return_code = machine->ReadRegister(4);
    machine->WriteRegister(2, return_code);

    // Wait the termination of all threads in the address space before halting the machine
    if (Process* process = currentThread->getProcess(); process != nullptr) { process->WaitForAllThreadsTerminate(); }

    interrupt->Halt();
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
            return; // HALT does not return, so we do not need to UpdatePC

        case SC_Exit:
            DEBUG('a', "Exit, initiated by user program.\n");
            handler_SC_exit();
            return; // EXIT does not return, so we do not need to UpdatePC

        case SC_PutChar:
            DEBUG('a', "PutChar exception.cc\n");
            handler_SC_putChar();
            break;

        case SC_PutString:
            DEBUG('a', "PutString exception.cc\n");
            handler_SC_putString();
            break;

        case SC_GetChar:
            DEBUG('a', "GetChar exception.cc\n");
            handler_SC_getChar();
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

        case SC_CreateThread:
            DEBUG('a', "CreateThread exception.cc\n");
            handle_SC_CreateThread();
            break;

        case SC_ExitThread:
            DEBUG('a', "ExitThread exception.cc\n");
            do_UserThreadExit();
            return;

        case SC_JoinThread:
            DEBUG('a', "JoinThread exception.cc\n");
            handle_SC_JoinThread();
            break;

        case SC_Sleep:
            DEBUG('a', "Sleep exception.cc\n");
            handle_SC_Sleep();
            break;

        case SC_SleepUntil:
            DEBUG('a', "SleepUntil exception.cc\n");
            handle_SC_SleepUntil();
            break;

        case SC_GetCurrentTick:
            DEBUG('a', "GetCurrentTick exception.cc\n");
            handle_SC_GetCurrentTick();
            break;

        case SC_SemInit:
            DEBUG('a', "SemInit  exception.cc\n");
            handle_SC_SemInit();
            break;

        case SC_SemP:
            DEBUG('a', "SemP  exception.cc\n");
            handle_SC_SemP();
            break;

        case SC_SemV:
            DEBUG('a', "SemV  exception.cc\n");
            handle_SC_SemV();
            break;

        case SC_SemDestroy:
            DEBUG('a', "SemDestroy  exception.cc\n");
            handle_SC_SemDestroy();
            break;

        case SC_SetMaxSemForProcess:
            DEBUG('a', "SetMaxSemForProcess  exception.cc\n");
            handle_SC_SetMaxSemForProcess();
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
