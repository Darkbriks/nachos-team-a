#include "userthread.h"
#include "syscall.h"
#include "process.h"
#include "stackmanager.h"
#include "thread.h"
#include "tls.h"

/**
 * @brief Start function for Nachos user thread.
 * This function initializes the user thread's registers and stack,
 * then starts executing the user program.
 *
 * @param f A pointer to a Param object containing thread parameters
 */
static void StartUserThread(const int f) {
    const Param *param = reinterpret_cast<Param *>(f);

    Thread* thread = currentThread;
    const AddrSpace* space = thread->getAddrSpace();

    if (!space) {
        DEBUG('t', "StartUserThread: No address space for thread TID=%d\n", thread->getTID());
        delete param;
        thread->Finish();
        return;
    }

    const unsigned int stackBase = thread->getUserStackBase();
    const unsigned int stackLimit = thread->getUserStackLimit();

    if (stackBase == 0) {
        DEBUG('t', "StartUserThread: No stack allocated for thread TID=%d\n", thread->getTID());
        delete param;
        thread->Finish();
        return;
    }

    int stackAddr = static_cast<int>(stackBase - 16);

    DEBUG('t', "StartUserThread: Initializing thread TID=%d with stack [0x%x - 0x%x], SP=0x%x\n",
          thread->getTID(), stackLimit, stackBase, stackAddr);

    const int prevPC = machine->ReadRegister(PCReg);

    space->InitRegisters();
    space->RestoreState();

    machine->WriteRegister(PCReg, param->get_function());
    machine->WriteRegister(NextPCReg, param->get_function() + 4);
    machine->WriteRegister(PrevPCReg, prevPC);
    machine->WriteRegister(StackReg, stackAddr);
    machine->WriteRegister(RetAddrReg, param->get_exit_addr());
    machine->WriteRegister(4, param->get_arg());

    if (const unsigned int tlsBase = thread->getTlsBase(); tlsBase != 0) {
        machine->WriteRegister(TLS_REGISTER, static_cast<int>(tlsBase));
        DEBUG('t', "StartUserThread: Set TLS base to 0x%x for thread TID=%d\n", tlsBase, thread->getTID());
    } else {
        DEBUG('t', "StartUserThread: No TLS base set for thread TID=%d\n", thread->getTID());
    }

    delete param;
    machine->Run();
}

int do_PthreadCreate(const int thread_ptr, const int attr_ptr, int start_routine, int arg, int wrapper_addr) {
    if (start_routine <= 0) { return -E_INVAL; }
    // TODO : Add more checks (addr is in valid user space, for example)

    Process *process = currentThread->getProcess();
    if (process == nullptr) { return -E_FAULT; }

    const AddrSpace *space = process->getSpace();
    if (space == nullptr) { return -E_FAULT; }

    StackManager *stackMgr = space->GetStackManager();
    if (stackMgr == nullptr) { return -E_FAULT; }

    posix_thread_attr_t attr;
    if (attr_ptr != 0) {
        for (unsigned int i = 0; i < sizeof(posix_thread_attr_t); i++) {
            if (machine->ReadMem(attr_ptr + i, 1, reinterpret_cast<int *>(&reinterpret_cast<char *>(&attr)[i])) == FALSE) {
                return -E_INVAL;
            }
        }
    } else {
        posix_thread_attr_init(&attr);
    }

    unsigned int stackBase, stackLimit;
    if (const int ret = stackMgr->AllocateStack(USER_STACK_DEFAULT_SIZE, &stackBase, &stackLimit); ret < 0) {
        DEBUG('t', "do_PthreadCreate: Failed to allocate stack for new thread\n");
        return ret;
    }

    Thread *thread = process->CreateThread("user_thread");
    if (!thread) {
        stackMgr->FreeStack(stackBase);
        DEBUG('t', "do_PthreadCreate: Failed to create thread object\n");
        return -E_NOMEM;
    }

    thread->setDetached(attr.detachstate == DETACHED);
    thread->setUserStack(stackBase, stackBase - stackLimit, stackLimit);
    stackMgr->MarkInUse(stackBase, thread->getTID());

    DEBUG('t', "do_PthreadCreate: created thread TID=%d with stack 0x%x-0x%x\n",
          thread->getTID(), stackLimit, stackBase);

    if (thread_ptr > 0) {
        posix_thread_t tid = thread->getTID();
        if (machine->WriteMem(thread_ptr, sizeof(posix_thread_t), static_cast<int>(tid)) == FALSE) {
            process->RemoveThread(thread);
            stackMgr->FreeStack(stackBase);
            return -E_INVAL;
        }
    }

    Param *param = new Param(start_routine, arg, wrapper_addr);
    thread->Fork(StartUserThread, reinterpret_cast<int>(param));

    return 0;
}

void do_PthreadExit(void *retval) {
    Thread* thread = currentThread;
    Process* process = thread->getProcess();

    DEBUG('t', "do_PthreadExit: thread TID=%d exiting with retval=0x%x\n", thread->getTID(), retval);

    thread->setReturnValue(retval);
    thread->setStatus(TERMINATED);

    if (process) {
        if (const AddrSpace* space = process->getSpace()) {
            StackManager* stackMgr = space->GetStackManager();
            if (const unsigned int stackBase = thread->getUserStackBase();stackMgr && stackBase != 0) {
                DEBUG('t', "do_PthreadExit: freeing stack 0x%x for thread TID=%d\n", stackBase, thread->getTID());
                stackMgr->FreeStack(stackBase);
                thread->setUserStack(0, 0, 0);
            }
        }
        process->ThreadTerminated(thread);
    }

    if (thread->isDetached()) {
        thread->getProcess()->RemoveThread(thread);
    }

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    thread->Joiner();
    thread->Sleep();

    (void)interrupt->SetLevel(oldLevel);
    ASSERT(FALSE);
}

int do_PthreadJoin(posix_thread_t tid, int retval_ptr) {
    Thread* other_thread = currentThread->getProcess()->FindThread(tid);

    if (!other_thread) { return -E_NOSPC; }
    if (other_thread == currentThread) { return -E_INVAL; }
    if (other_thread->isDetached()) { return -E_INVAL; }
    if (other_thread->hasJoiner()) { return -E_INVAL; }


    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    currentThread->setJoin(other_thread);
    other_thread->setJoiner(currentThread);
    (void)interrupt->SetLevel(oldLevel);

    other_thread->Join();

    if (retval_ptr >= 0) {
        void* retval = other_thread->getReturnValue();
        if (machine->WriteMem(retval_ptr, sizeof(void*), reinterpret_cast<int>(retval)) == FALSE) {
            return -E_INVAL;
        }
    }

    currentThread->getProcess()->RemoveThread(other_thread);

    return 0;
}

int do_PthreadDetach(posix_thread_t tid) {
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

void do_PthreadSelf(){
    Thread* thread = currentThread;
    if (!thread) { RETURN(-E_NOSPC);}
    RETURN(currentThread->getTID());
}

void handle_SC_PthreadCreate(){
    const int thread_ptr = machine->ReadRegister(4);
    const int attr_ptr = machine->ReadRegister(5);
    const int start_routine = machine->ReadRegister(6);
    const int arg = machine->ReadRegister(7);
    const int wrapper_addr = machine->ReadRegister(8);
    DEBUG('t', "SC_PthreadCreate called with thread_ptr=%d, attr_ptr=%d, start_routine=0x%x, arg=0x%x, wrapper_addr=0x%x\n",
          thread_ptr, attr_ptr, start_routine, arg, wrapper_addr);
    RETURN(do_PthreadCreate(thread_ptr, attr_ptr, start_routine, arg, wrapper_addr) );
}

void handle_SC_PthreadExit(){
    const int retval = machine->ReadRegister(4);
    do_PthreadExit( reinterpret_cast<void *>(retval) );
}

void handle_SC_PthreadJoin(){
    const int tid = machine->ReadRegister(4);
    const int retval_ptr = machine->ReadRegister(5);
    RETURN(do_PthreadJoin(tid, retval_ptr) );
}

void handle_SC_PthreadDetach(){
    const int tid = machine->ReadRegister(4);
    RETURN(do_PthreadDetach(tid) );
}

void handle_SC_PthreadSelf(){
    do_PthreadSelf();
}

void handle_SC_Pthread_attr_init() {
    const int attr_ptr = machine->ReadRegister(4);
    if (attr_ptr <= 0) { RETURN(-E_INVAL); }

    posix_thread_attr_t attr;
    posix_thread_attr_init(&attr);

    for (unsigned int i = 0; i < sizeof(posix_thread_attr_t); i++) {
        if (!machine->WriteMem(attr_ptr + i, 1, reinterpret_cast<char*>(&attr)[i])) {
            RETURN(-E_FAULT);
        }
    }
    RETURN(0);
}

void handle_SC_Pthread_attr_destroy() {
    RETURN(0);
}

void handle_SC_Pthread_attr_setdetachstate() {
    const int attr_ptr = machine->ReadRegister(4);
    const int detachstate = machine->ReadRegister(5);

    if (attr_ptr <= 0) { RETURN(-E_INVAL); }
    if (detachstate != JOINABLE && detachstate != DETACHED) { RETURN(-E_INVAL); }
    if (!machine->WriteMem(attr_ptr, sizeof(int), detachstate)) { RETURN(-E_FAULT); }

    RETURN(0);
}

void handle_SC_Pthread_attr_getdetachstate() {
    const int attr_ptr = machine->ReadRegister(4);
    const int result_ptr = machine->ReadRegister(5);

    if (attr_ptr <= 0 || result_ptr <= 0) { RETURN(-E_INVAL); }

    int detachstate;
    if (!machine->ReadMem(attr_ptr, sizeof(int), &detachstate)) { RETURN(-E_FAULT); }
    if (!machine->WriteMem(result_ptr, sizeof(int), detachstate)) { RETURN(-E_FAULT); }

    RETURN(0);
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

void handle_SC_GetTID() {
    const unsigned int tid = currentThread->getTID();
    DEBUG('t', "sys_get_tid: returning %d\n", tid);
    machine->WriteRegister(2, static_cast<int>(tid));
}