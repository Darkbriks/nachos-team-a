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

    Thread * firstThread = new Thread("main");
    if (executable != nullptr){
        this->space = new AddrSpace(executable, this);

        space->InitRegisters(); // set the initial register values
        space->RestoreState();  // load page table register
        firstThread->space = space;
        delete executable; // close file
scheduler->ReadyToRun(firstThread);
    }
    mainThread = firstThread;
    threadNumber = 1; // The main thread
    threadNumberLock = new Lock("thread number lock");
    threadExitSemaphore = new Semaphore("thread exit semaphore", 0);
    all_threads_addr = new LinkedList<Thread>();
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
        thread->space = nullptr;
        delete this; // Sorry, not very elegant, and a bit risky. Make it better later
    }
}

void Process::WaitForAllThreadsTerminate() {
    threadNumberLock->Acquire();
    while ( ! all_threads_addr->OnlyOneElement()) {
        const unsigned int remainingThreads = threadNumber - 1; // Exclude the main thread

        if (remainingThreads == 0) {
            break;
        }
        threadNumberLock->Release();

        DEBUG('t', "AddrSpace: Main thread waiting, %d child threads remaining\n", remainingThreads);
        threadExitSemaphore->P();
        threadNumberLock->Acquire();
    }
    threadNumberLock->Release();

    DEBUG('t', "AddrSpace: All child threads finished\n");
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

