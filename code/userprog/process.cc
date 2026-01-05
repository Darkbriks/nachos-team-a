#include "process.h"

#include "addrspace.h"
#include "bitmap_thread_safe.h"
#include "bitmap.h"
#include "thread.h"
#include "../threads/system.h"


BitMapThreadSafe* Process::all_process = new BitMapThreadSafe(MAX_PROCESS);

Process * Process::createProcess(OpenFile * executable){
    char * status_code = new char;
    Process *result = new Process(executable, status_code);
    if ( *status_code == -1){
        delete result;
        result = nullptr;
    }
    delete status_code;
    return result;
}


Process::Process(OpenFile * executable, char * status_code){
    *status_code=0;
    // all_threads = new BitMap(MAX_THREAD);

    const int tmp = all_process->Find();
    if (tmp == -1){
        *status_code=1;
        return;
    }

    PID = static_cast<unsigned int>(tmp);
    threadNumberLock = new Lock("thread number lock");
    threadExitSemaphore = new Semaphore("thread exit semaphore", 0);
    all_threads_addr = new LinkedList<Thread>();
    threads_bitmap = new BitMap(MAX_THREAD);

    threadNumber = 0; // The main thread
    Thread * firstThread = CreateThread(executable ? (char *) "main" : (char *) "kernel");
    if (executable != nullptr){
        this->space = new AddrSpace(executable);

        space->InitRegisters(); // set the initial register values
        space->RestoreState();  // load page table register
        delete executable; // close file
    }
    mainThread = firstThread;
}

Process::~Process(){
    // delete all_threads;
    delete threadNumberLock;
    delete threadExitSemaphore;
    delete all_threads_addr;
    delete threads_bitmap;
}

void Process::ThreadTerminated(Thread* thread){
    threadNumberLock->Acquire();
    const unsigned int remainingThreads = --threadNumber;
    threadNumberLock->Release();

    DEBUG('t', "Process: Thread %d terminated, now %d threads still running\n", thread->getTID(), remainingThreads);

    threadExitSemaphore->V();
}

void Process::RemoveThread(Thread * thread) {
    // ThreadTerminated must be called before, so threadNumber is already decreased
    threadNumberLock->Acquire();
    all_threads_addr->RemoveInList(thread);
    const unsigned int remainingThreads = threadNumber;
    threadNumberLock->Release();

    DEBUG('t', "Process: Removed thread %d, now %d threads\n", thread->getTID(), remainingThreads);

    if (thread != currentThread) {
        delete thread;
    } else {
        threadToBeDestroyed = thread;
    }

    if (remainingThreads == 0) {
        DEBUG('t', "Process: Last thread terminated, deleting AddrSpace\n");
        AddrSpace *spaceToDelete = this->space;
        this->space = nullptr;
        delete spaceToDelete;
        all_process->ClearThreadSafe(PID);
        // TODO: Scheduler should delete the process when the last thread exit
    }
}

void Process::WaitForAllThreadsTerminate() {
    threadNumberLock->Acquire();
    while (threadNumber > 1) { // Exclude the main thread
        threadNumberLock->Release();

        DEBUG('t', "Process: Main thread waiting, %d child threads remaining\n", threadNumber - 1);

        threadExitSemaphore->P();
        threadNumberLock->Acquire();
    }
    threadNumberLock->Release();

    // Now, all child threads are finished,
    // we can clear the thread list except the main thread
    threadNumberLock->Acquire();
    while (!all_threads_addr->IsEmpty()) {
        Thread* thread = all_threads_addr->RemoveFront();
        if (thread != mainThread) {
            delete thread;
        }
    }
    threadNumberLock->Release();

    DEBUG('t', "Process: All child threads finished\n");
}

bool search(Thread * T, unsigned int value){
    return T->getTID() == value;
}

Thread * Process::FindThread(unsigned int TID){
    return all_threads_addr->FindInList( std::function<bool (Thread *, unsigned int)> (search), TID );
}

Thread* Process::CreateThread(char * name) {
    const posix_thread_t tid = threads_bitmap->Find();
    if (tid == static_cast<posix_thread_t>(-1)) { return nullptr; }
    Thread * newThread = new Thread(name, this, tid);
    threadNumberLock->Acquire();
    threadNumber++;
    all_threads_addr->AddInList(newThread);
    threadNumberLock->Release();
    DEBUG('t', "Process %p : Added thread, now %d threads we create %d with name %s\n", this, threadNumber, newThread->getTID(), newThread->getName());
    return newThread;
}
