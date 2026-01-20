#include "userthread.h"

#include "kernelpanic.h"
#include "nos_errno.h"
#include "process.h"
#include "stackmanager.h"
#include "thread.h"
#include "nos_threads.h"

bool ValidateThreadArgs(const user_thread_args& args, const AddrSpace* space, const StackManager* stackMgr) {
    // entry : must be a valid user address, aligned to 4 bytes, and in code segment
    if (!space->IsUserAddress(args.entry) || (args.entry % 4 != 0) || !space->IsInCodeSegment(args.entry)) {
        DEBUG('t', "ValidateThreadArgs: Invalid entry point 0x%x\n", args.entry);
        return false;
    }

    // user_sp : must be 0 or a valid user stack pointer
    if (args.user_sp != 0 && !stackMgr->IsValidUserStackPointer(args.user_sp)) {
        DEBUG('t', "ValidateThreadArgs: Invalid user_sp 0x%x\n", args.user_sp);
        return false;
    }

    // tls_base : must be 0 or a valid TLS address
    if (args.tls_base != 0 && !space->IsValidTLS(args.tls_base)) {
        DEBUG('t', "ValidateThreadArgs: Invalid tls_base 0x%x\n", args.tls_base);
        return false;
    }

    return true;
}

void StartThread(int param){
    machine->Run();
}

void handle_SC_thread_create() {
    const unsigned args_ptr = machine->ReadRegister(4);

    GET_PROCESS_ADDRSPACE_STACKMANAGER();

    // Copy user_thread_args from user space to kernel space to avoid TOCTOU issues
    user_thread_args kargs;
    if (!CopyFromUserType<user_thread_args>(&kargs, args_ptr)) {
        DEBUG('t', "handle_SC_thread_create: Failed to copy user_thread_args from user space\n");
        RETURN(-E_FAULT);
    }

    VALIDATE_ARG(ValidateThreadArgs(kargs, space, stackMgr), E_INVAL);

    Thread* thread = process->CreateThread("user_thread", kargs.tls_base);
    VALIDATE_ARG(thread != nullptr, E_THREAD_LIMIT);

    thread->InitUserContext(kargs.entry, kargs.arg, kargs.user_sp);
    thread->Fork(StartThread, 0);
    RETURN(thread->getTID());
}

void handle_SC_thread_exit() {
    currentThread->setStatus(TERMINATED);

    Process* process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, E_FAULT);

    if (const ThreadStatus currentStatus = currentThread->getStatus(); currentStatus == BLOCKED || currentStatus == SLEEP) {
        DEBUG('t', "handle_SC_thread_exit: Cannot exit thread in state %d\n", currentStatus);
        RETURN(-E_INVAL);
    }

    process->ThreadTerminated(currentThread);
    process->RemoveThread(currentThread);

    const IntStatus oldLevel = interrupt->SetLevel(IntOff);
    currentThread->Sleep();
    (void)interrupt->SetLevel(oldLevel);
    KERNEL_PANIC("Returned from Thread::Sleep() in handle_SC_thread_exit (should never happen)");
}

void handle_SC_thread_self() {
    RETURN(static_cast<int>(currentThread->getTID()));
}

void handle_SC_thread_yield() {
    currentThread->Yield();
}
