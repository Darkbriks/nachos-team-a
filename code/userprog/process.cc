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
    all_threads = new BitMap(MAX_THREAD);

    const int tmp = all_process->Find();
    if (tmp == -1){
        *status_code=1;
        return;
    }

    PID = static_cast<unsigned int>(tmp);

    Thread * firstThread = new Thread("main");
    if (executable != nullptr){
        this->space = new AddrSpace(executable);

        space->InitRegisters(); // set the initial register values
        space->RestoreState();  // load page table register
        firstThread->space = space;
        delete executable; // close file
        scheduler->ReadyToRun(firstThread);
    }
    mainThread = firstThread;
}

Process::~Process(){
    delete all_threads;
}


