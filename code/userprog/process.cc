#include "process.h"
#include "bitmap_thread_safe.h"
#include "bitmap.h"


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
        this->space = new AddrSpace(executable, this);

        space->InitRegisters(); // set the initial register values
        space->RestoreState();  // load page table register
        firstThread->space = space;
        delete executable; // close file
        scheduler->ReadyToRun(firstThread);
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

    DEBUG('t', "AddrSpace: Removed thread, now %d threads\n", remainingThreads);

    threadExitSemaphore->V();

    if (remainingThreads == 0) {
        DEBUG('t', "AddrSpace: Last thread terminated, deleting AddrSpace\n");
        AddrSpace * spaceToDelete = thread->space;
        thread->space = nullptr;
        delete spaceToDelete;
        all_process->Clear(PID);
        // TODO: Scheduler should delete the process when the last thread exit
    }
}

void Process::WaitForAllThreadsTerminate() {
    threadNumberLock->Acquire();
    while (threadNumber > 1) { // Exclude the main thread
        threadNumberLock->Release();

        DEBUG('t', "AddrSpace: Main thread waiting, %d child threads remaining\n", threadNumber - 1);

        threadExitSemaphore->P();
        threadNumberLock->Acquire();
    }
    threadNumberLock->Release();

    DEBUG('t', "AddrSpace: All child threads finished\n");
}

bool search(Thread * T, unsigned int value){
    return T->getTID() == value;
}

Thread * Process::FindThread(unsigned int TID){
    return all_threads_addr->FindInList( std::function<bool (Thread *, unsigned int)> (search), TID );
}

Thread* Process::CreateThread(char * name){
    Thread * newThread = new Thread(name);
    threadNumberLock->Acquire();
    threadNumber++;
    all_threads_addr->AddInList(newThread);
    threadNumberLock->Release();
    DEBUG('t', "AddrSpace: Added thread, now %d threads\n", threadNumber);
    return newThread;
}

