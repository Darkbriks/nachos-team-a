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

#define CASE_HANDLER(syscall_name)                      \
    case SC_##syscall_name:                             \
        DEBUG('a', "%s exception.cc\n", #syscall_name); \
        handle_SC_##syscall_name();                     \
        break;

#define CASE_HANDLER_RETURN(syscall_name)               \
    case SC_##syscall_name:                             \
        DEBUG('a', "%s exception.cc\n", #syscall_name); \
        handle_SC_##syscall_name();                     \
        return;


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

void handle_SC_Halt() {
    DEBUG('a', "Shutdown, initiated by user program.\n");
    interrupt->Halt();
}

void handle_SC_Exit() {
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
        CASE_HANDLER_RETURN(Halt)
        CASE_HANDLER_RETURN(Exit)

        CASE_HANDLER(PutChar)
        CASE_HANDLER(PutString)
        CASE_HANDLER(GetChar)
        CASE_HANDLER(GetString)
        CASE_HANDLER(PutInt)
        CASE_HANDLER(GetInt)

        CASE_HANDLER(PthreadCreate)
        CASE_HANDLER_RETURN(PthreadExit)
        CASE_HANDLER(PthreadJoin)
        CASE_HANDLER(PthreadDetach)
        CASE_HANDLER(PthreadSelf)

        CASE_HANDLER(Pthread_attr_init)
        CASE_HANDLER(Pthread_attr_destroy)
        CASE_HANDLER(Pthread_attr_setdetachstate)
        CASE_HANDLER(Pthread_attr_getdetachstate)

        CASE_HANDLER(Sleep)
        CASE_HANDLER(SleepUntil)
        CASE_HANDLER(GetCurrentTick)

        CASE_HANDLER(SemInit)
        CASE_HANDLER(SemWait)
        CASE_HANDLER(SemPost)
        CASE_HANDLER(SemDestroy)
        CASE_HANDLER(SetMaxSemForProcess)

        default:
            printf("Unknow syscall :%d\n", type);
            ASSERT(FALSE);
            break;
    }

    // LB: Do not forget to increment the pc before returning!
    UpdatePC();
    // End of addition
}
