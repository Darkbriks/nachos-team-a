#include "userProcess.h"
#include "addrspace.h"
#include "synchconsole.h"
#include "copyright.h"
#include "synch.h"
#include "exception.h"
#include "syscall.h"
#include "system.h"
#include "process.h"
int do_ForkExec(){

    return 0;
}

extern void StartProcess(char* filename);

void handle_SC_ForkExec(){    
    int ptr_file_name = machine->ReadRegister(4);

    if (ptr_file_name <= 0) { RETURN(-E_FAULT); } // TODO Add more checks (addr is in valid user space, for example)

    int offset = 0;
    char buffer[MAX_PATH_SIZE];

    int size = copyStringFromMachine(ptr_file_name + offset, buffer, MAX_PATH_SIZE);
    DEBUG('p', "SC_ForkExec receive %s file_name with len %d\n", buffer, size);
    StartProcess(buffer);
    // OpenFile *executable = fileSystem->Open(buffer);
    // if (executable == NULL) {
    //     printf("Unable to open file %s\n", buffer);
    //     return;
    // }
    // Process * newProcess = Process::createProcess(executable);
    // if (newProcess == nullptr){
    //     ASSERT(FALSE);  
    // }
    // scheduler->ReadyToRun(newProcess->getMainThread());
    // DEBUG('p', "SC_ForkExec succeed for thread %s\n", currentThread->getName());
    // currentThread->Yield();
    RETURN(ptr_file_name);
}
