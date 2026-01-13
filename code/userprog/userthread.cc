#include "userthread.h"
#include "syscall.h"
#include "process.h"
#include "stackmanager.h"
#include "thread.h"
#include "tls.h"
#include "nos_threads.h"

void handle_SC_PthreadCreate(){
   /* user_thread_args args = {
        (unsigned int) machine->ReadRegister(6),
        (void *) machine->ReadRegister(7),
        0,
        0,
        0,
        0,
    };
    const int thread_ptr = machine->ReadRegister(4);
    machine->WriteRegister(4, (int) &args);
    handle_SC_thread_create();
    int TID = machine->ReadRegister(2);
    if (thread_ptr > 0){
        if (!CopyToUserType<int>(thread_ptr, &TID)) {
            DEBUG('t', "handle_SC_thread_create: Failed to copy user_thread_args from user space\n");
            RETURN(-E_FAULT);
        }
    }*/
    RETURN(0);
}

void handle_SC_PthreadExit(){
    handle_SC_thread_exit();
}

void handle_SC_PthreadJoin() {}

void handle_SC_PthreadDetach() {}

void handle_SC_SetTLS() {
    const int tlsPtr = machine->ReadRegister(4);

    DEBUG('t', "sys_set_tls: thread %s setting TLS to 0x%x\n", currentThread->getName(), tlsPtr);

    if (tlsPtr == 0) {
        DEBUG('t', "sys_set_tls: NULL pointer rejected\n");
        RETURN(-E_INVAL);
    }

    int testValue;
    if (!machine->ReadMem(tlsPtr, 4, &testValue)) {
        DEBUG('t', "sys_set_tls: Invalid TLS address 0x%x on read test\n", tlsPtr);
        RETURN(-E_FAULT);
    }

    if (!machine->WriteMem(tlsPtr, 4, testValue)) {
        DEBUG('t', "sys_set_tls: Invalid TLS address 0x%x on write test\n", tlsPtr);
        RETURN(-E_FAULT);
    }

    //currentThread->setTlsBase(static_cast<unsigned int>(tlsPtr));

    machine->WriteRegister(TLS_REGISTER, tlsPtr);

    DEBUG('t', "sys_set_tls: TLS set successfully to 0x%x\n", tlsPtr);
    RETURN(0);
}

void handle_SC_GetTLS() {
    const unsigned int tlsBase = currentThread->getUserTlsBase();
    DEBUG('t', "sys_get_tls: returning 0x%x\n", tlsBase);
    RETURN(static_cast<int>(tlsBase));
}

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

    Process *process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, E_FAULT);

    const AddrSpace *space = process->getSpace();
    VALIDATE_ARG(space != nullptr, E_FAULT);

    StackManager *stackMgr = space->GetStackManager();
    VALIDATE_ARG(stackMgr != nullptr, E_FAULT);

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

// TODO: A thread can be destroyed only if state isn't RUNNING, BLOCKED or SLEEP
void handle_SC_thread_exit() {
    currentThread->setStatus(TERMINATED);

    Process* process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, E_FAULT);

    process->ThreadTerminated(currentThread);
    process->RemoveThread(currentThread);

    const IntStatus oldLevel = interrupt->SetLevel(IntOff);
    currentThread->Sleep();
    (void)interrupt->SetLevel(oldLevel);
    ASSERT(FALSE);
}

void handle_SC_thread_self() {
    RETURN(static_cast<int>(currentThread->getTID()));
}

void handle_SC_thread_yield() {
    currentThread->Yield();
}
