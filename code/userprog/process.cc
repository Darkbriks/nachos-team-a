#include "process.h"

#include "addrspace.h"
#include "bitmap_thread_safe.h"
#include "bitmap.h"
#include "thread.h"
#include "../threads/system.h"

BitMapThreadSafe* Process::all_process = new BitMapThreadSafe(MAX_PROCESS);
int Process::activeProcessCount = 0;
Lock* Process::processCountLock = new Lock("process count lock");
LinkedList<Process>* Process::all_process_addr = new LinkedList<Process>();

void Process::freeAllStatic(){
    delete Process::processCountLock;
    delete Process::all_process;
    delete Process::all_process_addr;
}

bool Process::isLastActiveProcess() {
    processCountLock->Acquire();
    const bool result = (activeProcessCount == 1);
    processCountLock->Release();
    return result;
}

bool Process::doesProcessExist(int PID){
    return all_process->Test(PID);
}

bool searchProcess(Process* P, const unsigned int value) {
    return P->getPId() == value;
}

Process* Process::FindProcessByPID(const unsigned int PID) {
    if ( ! doesProcessExist(PID) ){
        return nullptr;
    }
    return all_process_addr->FindInList( std::function<bool (Process*, unsigned int)> (searchProcess), PID );
}

unsigned int Process::getCurrentNumberOfProcess(){
    int tmp = all_process->NumClearThreadSafe();
    if (tmp == -1){
        tmp = 0;
    }
    return MAX_PROCESS - tmp;
}

Process* Process::createProcess(OpenFile * executable) {
    if (Process::getCurrentNumberOfProcess() == MAX_PROCESS){
        return nullptr;
    }
    Process* result = nullptr;
    const auto status_code = new char;
    result = new Process(executable, status_code);
    if (*status_code == -1 && result != nullptr) {
        delete result;
        result = nullptr;
    }
    delete status_code;
    return result;
}


Process::Process(OpenFile * executable, char* return_code) {
    *return_code = 0;

    const int pid = all_process->Find();
    if (pid == -1) { *return_code = -1; return; }

    PID = static_cast<unsigned int>(pid);

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if (PID > 0) {
        processCountLock->Acquire();
        all_process_addr->AddInList(this);
        activeProcessCount++;
        DEBUG('p', "Process %d created, now %d active processes\n", PID, activeProcessCount);
        processCountLock->Release();
    } else {
        processCountLock->Acquire();
        all_process_addr->AddInList(this);
        processCountLock->Release();
        DEBUG('p', "Kernel process %d created (not counted)\n", PID);
    }
    interrupt->SetLevel(oldLevel);
    threadNumberLock = new Lock("thread number lock");
    threadExitSemaphore = new Semaphore("thread exit semaphore", 0);
    all_threads_addr = new LinkedList<Thread>();
    threads_bitmap = new BitMap(MAX_THREAD);

    exitCode = 0;
    threadNumber = 0; // The main thread
    Thread * firstThread = CreateThread(executable ? "main" : "kernel");
    this->space = nullptr;

    if (executable != nullptr) {
        this->space = new AddrSpace(executable);
        delete executable; // close file
    }

    mainThread = firstThread;
    ancestor = currentThread ? currentThread->getProcess() : nullptr;
    if (ancestor != nullptr ){
        DEBUG('p', "Process %d have process %d for ancestor\n", PID, ancestor->getPId());
        ancestorSem = new Semaphore("Child sem for join between process", 0);
    } else {
        DEBUG('p', "Process %d don't have process for ancestor\n", PID);
    }
}

void Process::AncestorWait(){
    ancestorSem->P();
}

void Process::AncestorSigChild(){
    ancestorSem->V();
}

void Process::WaitForChild(Process* child){
    child->AncestorWait();
}

Process::~Process() {
    DEBUG('p', "Process %d destructor called\n", PID);

    if (space != nullptr) {
        DEBUG('p', "Process %d: Deleting AddrSpace\n", PID);
        delete space;
        space = nullptr;
    }

    delete threadNumberLock;
    delete threadExitSemaphore;
    delete all_threads_addr;
    delete threads_bitmap;
    all_process->ClearThreadSafe(static_cast<int>(PID));
    if (PID > 0) {
        processCountLock->Acquire();
        activeProcessCount--;
        all_process_addr->RemoveInList(this);
        const int remaining = activeProcessCount;
        DEBUG('p', "Process %d destroyed, %d active processes remaining\n", PID, remaining);
        delete ancestorSem;
        processCountLock->Release();

        if (remaining == 0) {
            DEBUG('p', "Last user process terminated, halting machine\n");
            interrupt->Halt();
        }
    } else {
        DEBUG('p', "Kernel process %d destroyed\n", PID);
    }
}

void Process::KillAllThreads(){
    Thread * thread = nullptr;
    while ( ( thread = all_threads_addr->RemoveFront()) != nullptr){
        delete thread;
    }
}

void Process::ThreadTerminated(Thread* thread) {
    threadNumberLock->Acquire();
    const unsigned int remainingThreads = --threadNumber;
    if (threadNumber == 0){
        if (Process::isLastActiveProcess()) {
            delete currentThread->getProcess();
            interrupt->Halt();
        }

        ASSERT(processToBeDestroyed == nullptr);
        processToBeDestroyed = currentThread->getProcess();

        currentThread->Finish();

        ASSERT(FALSE);
    }
    threadNumberLock->Release();

    DEBUG('t', "Process: Thread %d terminated, now %d threads still running\n", thread->getTID(), remainingThreads);

    threadExitSemaphore->V();
}

void Process::RemoveThread(Thread * thread) const {
    // ThreadTerminated must be called before, so threadNumber is already decreased
    threadNumberLock->Acquire();
    all_threads_addr->RemoveInList(thread);
    const unsigned int remainingThreads = threadNumber;
    threadNumberLock->Release();

    DEBUG('t', "Process: Removed thread %d, now %d threads\n", thread->getTID(), remainingThreads);

    if (thread != currentThread) { delete thread; }
    else { threadToBeDestroyed = thread; }
}

void Process::WaitForAllThreadsTerminate() const {
    threadNumberLock->Acquire();
    while (threadNumber > 1) { // Exclude the main thread
        threadNumberLock->Release();

        DEBUG('t', "Process: Main thread waiting, %d child threads remaining\n", threadNumber - 1);

        threadExitSemaphore->P();
        threadNumberLock->Acquire();
    }
    threadNumberLock->Release();

    DEBUG('t', "Process: All child threads finished\n");
}

bool search(Thread * T, const unsigned int value) {
    return T->getTID() == value;
}

Thread * Process::FindThread(const unsigned int TID) const {
    return all_threads_addr->FindInList( std::function<bool (Thread *, unsigned int)> (search), TID );
}

Thread* Process::CreateThread(const char * name) {
    const posix_thread_t tid = threads_bitmap->Find();
    if (tid == static_cast<posix_thread_t>(-1)) { return nullptr; }
    auto* newThread = new Thread(name, this, tid);
    threadNumberLock->Acquire();
    threadNumber++;
    all_threads_addr->AddInList(newThread);
    threadNumberLock->Release();
    DEBUG('t', "Process %p : Added thread, now %d threads we create %d with name %s\n", this, threadNumber, newThread->getTID(), newThread->getName());
    return newThread;
}
