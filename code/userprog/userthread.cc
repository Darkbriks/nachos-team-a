#include "userthread.h"
#include "syscall.h"
#include "process.h"
#include "thread.h"

/**
 * @brief Start function for Nachos user thread.
 * This function initializes the user thread's registers and stack,
 * then starts executing the user program.
 *
 * @param f A pointer to a Param object containing thread parameters
 */
static void StartUserThread(const int f) {
    const Param *param = reinterpret_cast<Param *>(f);

    // Initialize the stack pointer with 2 pages below the main thread stack
    // TODO: Fix this when step 4 is done
    const unsigned int numPages = currentThread->getAddrSpace()->GetNumPages();
    const unsigned int stackSize = 2 * PageSize; // User Thread or AddrSpace defined stack size ?
    const int stackAddr = static_cast<int>(numPages * PageSize - (currentThread->getTID() + 1) * stackSize);

    if (stackAddr < 0) {
        DEBUG('t', "StartUserThread: Invalid stack address 0x%x for thread TID=%d\n", stackAddr, currentThread->getTID());
        delete param;
        currentThread->Finish();
        return;
    }

    const int prevPC = machine->ReadRegister(PCReg);

    currentThread->getAddrSpace()->InitRegisters();
    currentThread->getAddrSpace()->RestoreState();

    machine->WriteRegister(PCReg, param->get_function());
    machine->WriteRegister(NextPCReg, param->get_function() + 4);
    machine->WriteRegister(PrevPCReg, prevPC);
    machine->WriteRegister(StackReg, stackAddr);
    machine->WriteRegister(RetAddrReg, param->get_exit_addr());
    machine->WriteRegister(4, param->get_arg());

    delete param;
    machine->Run();
}

int do_PthreadCreate(const int thread_ptr, const int attr_ptr, int start_routine, int arg, int wrapper_addr) {
    if (start_routine <= 0) { return -E_INVAL; }
    // TODO : Add more checks (addr is in valid user space, for example)

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

    Thread *thread = currentThread->getProcess()->CreateThread((char *)"user_thread");
    if (!thread) { return -E_NOMEM; }

    thread->setDetached(attr.detachstate == DETACHED);

    if (thread_ptr > 0) {
        posix_thread_t tid = thread->getTID();
        if (machine->WriteMem(thread_ptr, sizeof(posix_thread_t), static_cast<int>(tid)) == FALSE) {
            currentThread->getProcess()->RemoveThread(thread);
            return -E_INVAL;
        }
    }

    Param *param = new Param(start_routine, arg, wrapper_addr);
    DEBUG('t', "Creating user thread TID=%d, start_routine=0x%x, arg=0x%x, wrapper_addr=0x%x\n",
          thread->getTID(), param->get_function(), param->get_arg(), param->get_exit_addr());
    thread->Fork(StartUserThread, reinterpret_cast<int>(param));

    return 0;
}

void do_PthreadExit(void *retval) {
    Thread* thread = currentThread;

    thread->setReturnValue(retval);
    thread->setStatus(TERMINATED);

    thread->Joiner();
    thread->getProcess()->ThreadTerminated(thread);

    if (thread->isDetached()) {
        thread->getProcess()->RemoveThread(thread);
    }

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
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

    currentThread->setJoin(other_thread);
    other_thread->setJoiner(currentThread);

    //if (!other_thread->isTerminated()) {
        other_thread->Join();
    //}

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
