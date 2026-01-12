#include "userthread.h"
#include "syscall.h"
#include "process.h"
#include "stackmanager.h"
#include "thread.h"
#include "tls.h"
#include "nos_threads.h"


int do_PthreadDetach(tid_t tid) {
    Thread* thread = currentThread->getProcess()->FindThread(tid);

    if (!thread) { return -E_NOSPC; }
    if (thread->isDetached()) { return -E_INVAL; }
    if (thread->hasJoiner()) { return -E_INVAL; }

    thread->setDetached(true);

    if (thread->isTerminated()) {
        thread->getProcess()->RemoveThread(thread);
    }

    return 0;
}


void handle_SC_PthreadCreate(){
    user_thread_args args = {
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
    if (!CopyToUserType<int>(thread_ptr, &TID)) {
        DEBUG('t', "handle_SC_thread_create: Failed to copy user_thread_args from user space\n");
        RETURN(-E_FAULT);
    }
    RETURN(0);
}

void handle_SC_PthreadExit(){
    handle_SC_thread_exit();
}

void handle_SC_PthreadJoin(){
    handle_SC_thread_join();
}

void handle_SC_PthreadDetach(){
    const int tid = machine->ReadRegister(4);
    RETURN(do_PthreadDetach(tid) );
}

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

    currentThread->setTlsBase(static_cast<unsigned int>(tlsPtr));

    machine->WriteRegister(TLS_REGISTER, tlsPtr);

    DEBUG('t', "sys_set_tls: TLS set successfully to 0x%x\n", tlsPtr);
    RETURN(0);
}

void handle_SC_GetTLS() {
    const unsigned int tlsBase = currentThread->getTlsBase();
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

    // TODO: flags
    // TODO: clear_tid

    return true;
}

void StartThread(int param){
    machine->Run();
}

void handle_SC_thread_create() {
    user_thread_args kargs = *(user_thread_args *)machine->ReadRegister(4);

    Process *process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, E_FAULT);

    const AddrSpace *space = process->getSpace();
    VALIDATE_ARG(space != nullptr, E_FAULT);

    StackManager *stackMgr = space->GetStackManager();
    VALIDATE_ARG(stackMgr != nullptr, E_FAULT);

    // Copy user_thread_args from user space to kernel space to avoid TOCTOU issues
    // user_thread_args kargs;
    // if (!CopyFromUserType<user_thread_args>(&kargs, args_ptr)) {
    //     DEBUG('t', "handle_SC_thread_create: Failed to copy user_thread_args from user space\n");
    //     RETURN(-E_FAULT);
    // }
    //
    VALIDATE_ARG(ValidateThreadArgs(kargs, space, stackMgr), E_INVAL);

    Thread* thread = process->CreateThread("user_thread");
    VALIDATE_ARG(thread != nullptr, E_THREAD_LIMIT);

    thread->setDetached((kargs.flags & USER_THREAD_FLAG_DETACHED) != 0);
    thread->setTlsBase(kargs.tls_base);
    thread->setClearChildTid(reinterpret_cast<int*>(kargs.clear_tid));

    // Allocate stack if user_sp is 0
    // TODO: Force user to provide a stack ?
    if (kargs.user_sp == 0) {
        unsigned int stackBase, stackLimit;
        const int ret = stackMgr->AllocateStack(USER_STACK_DEFAULT_SIZE, &stackBase, &stackLimit);
        if (ret < 0) {
            process->RemoveThread(thread);
            DEBUG('t', "handle_SC_thread_create: Failed to allocate stack for new thread\n");
            RETURN(ret);
        }
        thread->setUserStack(stackBase, USER_STACK_DEFAULT_SIZE, stackLimit);
        stackMgr->MarkInUse(stackBase, thread->getTID());
        kargs.user_sp = stackBase; // Initial SP
        DEBUG('t', "handle_SC_thread_create: Allocated stack 0x%x-0x%x for thread TID=%d\n",
              stackLimit, stackBase, thread->getTID());
    } else {
        DEBUG('t', "handle_SC_thread_create: Using provided user_sp 0x%x for thread TID=%d\n",
              kargs.user_sp, thread->getTID());
    }

    thread->InitUserContext(kargs.entry, reinterpret_cast<unsigned int>(kargs.arg), kargs.user_sp);
    thread->Fork(StartThread, 0); // no parameter

    RETURN(thread->getTID());
}

void handle_SC_thread_exit() {
    const auto retval = reinterpret_cast<void*>(machine->ReadRegister(4));

    Thread* thread = currentThread;
    Process* process = thread->getProcess();

    DEBUG('t', "do_thread_exit: thread TID=%d exiting with retval=0x%x\n", thread->getTID(), retval);

    thread->setReturnValue(retval);
    thread->setStatus(TERMINATED);

    if (int* uaddr = thread->getClearChildTid(); uaddr != nullptr) {
        if (machine->WriteMem(reinterpret_cast<int>(uaddr), sizeof(int), 0) == FALSE) {
            DEBUG('t', "do_thread_exit: Failed to clear child TID at user address 0x%x\n", reinterpret_cast<int>(uaddr));
        } else {
            DEBUG('t', "do_thread_exit: Cleared child TID at user address 0x%x\n", reinterpret_cast<int>(uaddr));
        }

        // TODO: Implement futex wake ?
    }

    if (process) {
        if (const AddrSpace* space = process->getSpace()) {
            StackManager* stackMgr = space->GetStackManager();
            if (const unsigned int stackBase = thread->getUserStackBase(); stackMgr && stackBase != 0) {
                DEBUG('t', "do_thread_exit: freeing stack 0x%x for thread TID=%d\n", stackBase, thread->getTID());
                stackMgr->FreeStack(stackBase);
                thread->clearUserStack();
            }
        }
        process->ThreadTerminated(thread);
    }

    if (thread->isDetached()) {
        thread->getProcess()->RemoveThread(thread);
    }

    const IntStatus oldLevel = interrupt->SetLevel(IntOff);
    thread->Joiner();
    thread->Sleep();
    (void)interrupt->SetLevel(oldLevel);
    ASSERT(FALSE);
}

void handle_SC_thread_join() {
    const auto tid = static_cast<tid_t>(machine->ReadRegister(4));
    const int retval_ptr = machine->ReadRegister(5);

    Thread* self = currentThread;
    Process* process = self->getProcess();
    VALIDATE_ARG(process != nullptr, E_FAULT);

    Thread* target = process->FindThread(tid);
    VALIDATE_ARG(target != nullptr, E_NOENT);
    VALIDATE_ARG(target != self, E_INVAL);
    VALIDATE_ARG(!target->isDetached(), E_INVAL);
    VALIDATE_ARG(!target->hasJoiner(), E_BUSY);

    const IntStatus oldLevel = interrupt->SetLevel(IntOff);
    self->setJoin(target);
    target->setJoiner(self);
    (void)interrupt->SetLevel(oldLevel);

    target->Join();

    if (retval_ptr) {
        void* retval = target->getReturnValue();
        CopyToUserType<void*>(retval_ptr, &retval);
    }

    process->RemoveThread(target);
    RETURN(E_SUCCESS);
}

void handle_SC_thread_self() {
    RETURN(static_cast<int>(currentThread->getTID()));
}

void handle_SC_thread_yield() {
    currentThread->Yield();
}
