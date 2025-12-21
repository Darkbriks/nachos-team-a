#include "process.h"

#include "addrspace.h"
#include "bitmap_thread_safe.h"
#include "bitmap.h"
#include "thread.h"


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

    threadNumber = 0; // The main thread
    Thread * firstThread = CreateThread((char *) "main");
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
}

void Process::RemoveThread(Thread * thread) {
    threadNumberLock->Acquire();
    const unsigned int remainingThreads = --threadNumber;
    all_threads_addr->RemoveInList(thread);
    threadNumberLock->Release();

    DEBUG('t', "Process: Removed thread %d, now %d threads\n", thread->getTID(), remainingThreads);

    threadExitSemaphore->V();

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

    DEBUG('t', "Process: All child threads finished\n");
}

bool search(Thread * T, unsigned int value){
    return T->getTID() == value;
}

Thread * Process::FindThread(unsigned int TID){
    return all_threads_addr->FindInList( std::function<bool (Thread *, unsigned int)> (search), TID );
}

Thread* Process::CreateThread(char * name){
    Thread * newThread = new Thread(name, this);
    threadNumberLock->Acquire();
    threadNumber++;
    all_threads_addr->AddInList(newThread);
    threadNumberLock->Release();
    DEBUG('t', "Process: Added thread, now %d threads\n", threadNumber);
    return newThread;
}

