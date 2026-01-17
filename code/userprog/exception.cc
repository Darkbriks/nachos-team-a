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
#include "futex.h"
#include "userIO.h"
#include "usernetwork.h"
#include "userSbrk.h"
#include "userprocess.h"
#include "userSleep.h"
#include "userSem.h"

#define CASE_HANDLER(syscall_name)                      \
    case SC_##syscall_name:                             \
        DEBUG('e', "%s exception.cc\n", #syscall_name); \
        handle_SC_##syscall_name();                     \
        break;

#define CASE_HANDLER_RETURN(syscall_name)               \
    case SC_##syscall_name:                             \
        DEBUG('e', "%s exception.cc\n", #syscall_name); \
        handle_SC_##syscall_name();                     \
        return;

#define EXCEPTION_ERROR(error_name, error_code) \
    if (process == nullptr) { currentThread->Finish(); return; }   \
    process->KillAllThreads(false);   \
    printf(error_name);   \
    if (Process::isLastActiveProcess()) {   \
        ASSERT(false) /* To hit gdb */ \
        interrupt->Halt();   \
    }   \
    process->setExitCode(-error_code);   \
    process->AncestorSigChild();   \
    currentThread->Finish();  \
    ASSERT(false) /* To hit gdb */ \
    break;



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
    DEBUG('e', "Shutdown, initiated by user program.\n");
    interrupt->Halt();
}

void handle_SC_Exit() {
    const int return_code = machine->ReadRegister(4);
    machine->WriteRegister(2, return_code);

    Process* process = currentThread->getProcess();

    if (process == nullptr) { currentThread->Finish(); return; }

    DEBUG('p', "Exit: Process %d (thread %d) exiting with code %d\n", process->getPId(), currentThread->getTID(), return_code);

    process->WaitForAllThreadsTerminate();

    if (Process::isLastActiveProcess()) {
        delete process;
        interrupt->Halt();
    }

    ASSERT(processToBeDestroyed == nullptr);
    process->setExitCode(return_code);
    process->AncestorSigChild();

    currentThread->Finish();

    ASSERT(FALSE);
}

void handle_Error(ExceptionType which, int type){
    Process* process = currentThread->getProcess();

    DEBUG('e', "Exception: %d, type: %d in process %d (thread %d)\n", which, type,
          process ? process->getPId() : -1, currentThread->getTID());

    switch (which){
        case AddressErrorException:
            EXCEPTION_ERROR("Erreur de segmentation (core dumped)\n", 3);
        case ReadOnlyException:
            EXCEPTION_ERROR("Write on a read-only address\n", 4);
        case PageFaultException:
            EXCEPTION_ERROR("Access on an invalid address . Allocate more memory\n", 5);
        default:
            EXCEPTION_ERROR("Not yet manage exception, just kill the process\n", 1);
        }
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2);

    if (which != SyscallException){
        handle_Error(which, type);
        return;
    }

    switch (type){
        CASE_HANDLER_RETURN(Halt)

        CASE_HANDLER(PutChar)
        CASE_HANDLER(PutString)
        CASE_HANDLER(GetChar)
        CASE_HANDLER(GetString)
        CASE_HANDLER(PutInt)
        CASE_HANDLER(GetInt)

        CASE_HANDLER(Sleep)
        CASE_HANDLER(SleepUntil)
        CASE_HANDLER(GetCurrentTick)

        CASE_HANDLER(SemInit)
        CASE_HANDLER(SemWait)
        CASE_HANDLER(SemPost)
        CASE_HANDLER(SemDestroy)
        CASE_HANDLER(SetMaxSemForProcess)

        CASE_HANDLER(ForkExec)
        CASE_HANDLER(ForkJoin)
        CASE_HANDLER(ForkSelf)
        CASE_HANDLER_RETURN(Exit)

        CASE_HANDLER(Sbrk);
        CASE_HANDLER(mmap);
        CASE_HANDLER(munmap);

        CASE_HANDLER(thread_create);
        CASE_HANDLER_RETURN(thread_exit);
        CASE_HANDLER(thread_self);
        CASE_HANDLER(thread_yield);

        CASE_HANDLER(futex_wait);
        CASE_HANDLER(futex_wake);
        CASE_HANDLER(atomic_cmpxchg);
        CASE_HANDLER(atomic_store);
        CASE_HANDLER(atomic_load);

        CASE_HANDLER(connect);
        CASE_HANDLER(listen);
        CASE_HANDLER(accept);
        CASE_HANDLER(sendto);
        CASE_HANDLER(recvfrom);
        CASE_HANDLER(close);

        default:
            Process * process = currentThread->getProcess();
            EXCEPTION_ERROR("Unknow syscall", 2);
    }

    // LB: Do not forget to increment the pc before returning!
    UpdatePC();
    // End of addition
}
